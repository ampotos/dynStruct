#include "dr_api.h"
#include "dr_tools.h"
#include "../includes/tree.h"
#include "../includes/elf.h"
#include "../includes/utils.h"
#include "../includes/sym.h"

tree_t	*plt_tree = NULL;


void get_tmp_data_32(Elf32_Ehdr *elf_hdr,
		     sect_tmp_data *tmp_data, char *sect_name)
{
  Elf32_Shdr	*sect;
  Elf32_Phdr	*seg;
  char		*string_table;

  string_table = (void *)elf_hdr +
    ((Elf32_Shdr *)((void *)elf_hdr + elf_hdr->e_shoff +
		    elf_hdr->e_shstrndx * elf_hdr->e_shentsize))->sh_offset;

  for (int idx_sect = 0; idx_sect < elf_hdr->e_shnum; idx_sect++)
    {
      sect = (Elf32_Shdr *)((void *)elf_hdr +
			    elf_hdr->e_shoff + elf_hdr->e_shentsize * idx_sect);
      if (!ds_strcmp(string_table + sect->sh_name, sect_name))
	{
	  tmp_data->sect_offset = sect->sh_offset;
	  tmp_data->sect_size = sect->sh_size;
	  break;
	}
    }

  if (!tmp_data->sect_offset && !tmp_data->sect_size)
    return;
  
  for (int idx_seg = 0; idx_seg < elf_hdr->e_phnum; idx_seg++)
    {
      seg = (Elf32_Phdr *)((void *)elf_hdr +
			   elf_hdr->e_phoff + elf_hdr->e_phentsize * idx_seg);
      if (seg->p_offset <= tmp_data->sect_offset &&
	  seg->p_offset + seg->p_filesz > tmp_data->sect_offset)
	{
	  tmp_data->size_seg = (seg->p_memsz / DYNAMO_ALIGN) * DYNAMO_ALIGN +
	    (seg->p_memsz % DYNAMO_ALIGN ? DYNAMO_ALIGN : 0);
	  tmp_data->seg_perm = seg->p_flags;
	  tmp_data->sect_offset -= seg->p_offset;
	  break;
	}
    }
}

void get_tmp_data_64(Elf64_Ehdr *elf_hdr,
		     sect_tmp_data *tmp_data, char *sect_name)
{
  Elf64_Shdr	*sect;
  Elf64_Phdr	*seg;
  char		*string_table;

  string_table = (void *)elf_hdr +
    ((Elf64_Shdr *)((void *)elf_hdr + elf_hdr->e_shoff +
		    elf_hdr->e_shstrndx * elf_hdr->e_shentsize))->sh_offset;

  for (int idx_sect = 0; idx_sect < elf_hdr->e_shnum; idx_sect++)
    {
      sect = (Elf64_Shdr *)((void *)elf_hdr +
			    elf_hdr->e_shoff + elf_hdr->e_shentsize * idx_sect);
      if (!ds_strcmp(string_table + sect->sh_name, sect_name))
	{
	  tmp_data->sect_offset = sect->sh_offset;
	  tmp_data->sect_size = sect->sh_size;
	  break;
	}
    }

  if (!tmp_data->sect_offset && !tmp_data->sect_size)
    return;

  for (int idx_seg = 0; idx_seg < elf_hdr->e_phnum; idx_seg++)
    {
      seg = (Elf64_Phdr *)((void *)elf_hdr +
			   elf_hdr->e_phoff + elf_hdr->e_phentsize * idx_seg);
      if (seg->p_offset <= tmp_data->sect_offset &&
	  seg->p_offset + seg->p_filesz > tmp_data->sect_offset &&
	  seg->p_type == PT_LOAD)
	{
	  tmp_data->size_seg = (seg->p_memsz / DYNAMO_ALIGN) * DYNAMO_ALIGN +
	    (seg->p_memsz % DYNAMO_ALIGN ? DYNAMO_ALIGN : 0);
	  tmp_data->seg_perm = seg->p_flags;
	  break;
	}
    }
}

module_segment_data_t *find_load_section(const module_data_t *mod,
				     sect_tmp_data *tmp_data, char *sect_name)
{
  file_t	file;
  uint64	file_sz;
  void		*map_file;

  file = dr_open_file(mod->full_path, DR_FILE_READ);
  dr_file_size(file, &file_sz);

  DR_ASSERT_MSG((map_file = dr_map_file(file, (size_t *)(&file_sz), 0, NULL,
  					DR_MEMPROT_READ, DR_MAP_PRIVATE)),
  		"Error mapping file in plt search\n");

  if (*((uint32_t *)map_file) != *((uint32_t *)ELFMAG))
    {
      // this is not an elf file
      dr_close_file(file);
      return NULL;
    }
  
  if (((char*)map_file)[4] == ELFCLASS32)
    get_tmp_data_32(map_file, tmp_data, sect_name);
  else if (((char*)map_file)[4] == ELFCLASS64)
    get_tmp_data_64(map_file, tmp_data, sect_name);
  else
    {
      dr_close_file(file);
      return NULL;
    }

  dr_unmap_file(map_file, file_sz);
  dr_close_file(file);

  if (!tmp_data->sect_offset && !tmp_data->sect_size)
    return NULL;

  for (uint idx_seg = 0; idx_seg < mod->num_segments; idx_seg++)
    {
      if ((size_t)mod->segments[idx_seg].end -
	  (size_t)mod->segments[idx_seg].start == tmp_data->size_seg &&
	  (mod->segments[idx_seg].prot == tmp_data->seg_perm))
	return mod->segments + idx_seg;
    }

  return NULL;
}

void *get_got_from_plt(void *plt, void *drc)
{
  void		*jmp_pc = dr_app_pc_for_decoding(decode_next_pc(drc, plt));
  instr_t       *instr = instr_create(drc);
  void		*got = NULL;

  instr_init(drc, instr);
  if (!decode(drc, jmp_pc, instr))
    {
      dr_printf("Decode of instruction at %p failed\n", jmp_pc);
      instr_destroy(drc, instr);
      return NULL;
    }
  if (instr_get_opcode(instr) == OP_jmp_ind)
#ifdef BUILD_64
    instr_get_rel_addr_target(instr, (app_pc *)(&got));
#else
    got = opnd_get_addr(instr_get_target(instr));
#endif
  instr_destroy(drc, instr);

  return got + sizeof(void *);
}

void add_plt(const module_data_t *mod, void *got, void *drcontext)
{
  sect_tmp_data		tmp_data_plt;
  tree_t		*new_node;
  module_segment_data_t	*seg_plt;

  // if the path to the module is not present we can't get the file
  if (!(mod->full_path))
    return;
  
  if (!(seg_plt = find_load_section(mod, &tmp_data_plt, PLT_NAME)))
    return;
  if (!(new_node = dr_global_alloc(sizeof(*new_node))))
    {
      dr_printf("Can't alloc\n");
      return;
    }
  new_node->min_addr = seg_plt->start + tmp_data_plt.sect_offset;
  new_node->high_addr = new_node->min_addr + tmp_data_plt.sect_size;

  // we store the addr of the got of the module
  // with that we can find where we are going to jump when we are in the plt
  if (!got)
    {
      if (!(got = get_got_from_plt(new_node->min_addr, drcontext)))
	{
	  dr_global_free(new_node, sizeof(*new_node));
	  return;
	}
    }
  else
    got += 3 * sizeof(void *);

  new_node->data = got;

  add_to_tree(&plt_tree, new_node);
}


void remove_plt(const module_data_t *mod)
{
  sect_tmp_data		tmp_data;
  module_segment_data_t	*seg;

  if (!(seg = find_load_section(mod, &tmp_data, PLT_NAME)))
    return;

  del_from_tree(&plt_tree, seg->start + tmp_data.sect_offset, NULL);
}
