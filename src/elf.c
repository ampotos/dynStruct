#include "dr_api.h"
#include "dr_tools.h"
#include "../includes/tree.h"
#include "../includes/elf.h"
#include "../includes/utils.h"

tree_t	*plt_tree = NULL;

void get_plt(const module_data_t *mod)
{
  file_t	module;
  char		nident[EI_NIDENT];
  int		nb_sect = 0;
  unsigned long	str_tab_off; 
  char		sect_name[5];
  tree_t	*new_node;

  if ((module = dr_open_file(mod->full_path, DR_FILE_READ)) == INVALID_FILE)
    return ;
  if (dr_read_file(module, nident, EI_NIDENT) == EI_NIDENT &&
      !ds_strncmp(nident, ELFMAG, ds_strlen(ELFMAG)))
    {
      if (nident[4] == ELFCLASS64)
	{
	  Elf64_Ehdr	header;
	  Elf64_Shdr	section;

	  if (dr_read_file(module, &header, sizeof(header)) == sizeof(header))
	    {
	      // get string table
	      dr_file_seek(module, header.e_shoff + header.e_shentsize * header.e_shstrndx, DR_SEEK_SET);
	      if (dr_read_file(module, &section, header.e_shentsize) !=
		  header.e_shentsize)
		{
		  dr_close_file(module);
		  return ;
		}
	      str_tab_off = section.sh_offset;

	      // search for the ".plt" section
	      while (nb_sect < header.e_shnum)
		{
		  dr_file_seek(module,
			       header.e_shoff + header.e_shentsize * nb_sect,
			       DR_SEEK_SET);
		  if (dr_read_file(module, &section, header.e_shentsize) !=
		      header.e_shentsize)
		    {
		      dr_close_file(module);
		      return ;
		    }
		  // get section name
		  dr_file_seek(module, str_tab_off + section.sh_name, DR_SEEK_SET);
		  if (dr_read_file(module, sect_name, ds_strlen(PLT_NAME)) !=
                      (ssize_t)ds_strlen(PLT_NAME))
                    {
		      dr_close_file(module);
		      return ;
		    }
		  if (!ds_strncmp(sect_name, PLT_NAME, ds_strlen(PLT_NAME) + 1))
		    {
		      // ".plt" section found
		      if ((new_node = dr_global_alloc(sizeof(*new_node))))
			{
			  /* ds_memset(new_node, 0, sizeof(*new_node)); */
			  /* // todo : find plt addr and refoctor this sheet */
			  /* new_node->high_addr = ;//end of plt; */
			  /* new_node->min_addr = ;//start of plt; */
			  /* // we don't need to store specific data */
			  /* // we just want to know if the pc is in the plt */
			  /* new_node->data = (void *)IN_PLT; */
			  /* add_to_tree(&plt_tree, new_node); */
			  dr_printf("%p  0x%x\n", section.sh_addr, section.sh_size);
			}
		      dr_close_file(module);
		      return ;
		    }

		  nb_sect++;
		}
	    }
	}
      else if (nident[4] == ELFCLASS32)
	{
	  Elf32_Ehdr	header;
	  Elf32_Shdr	section;

	  if (dr_read_file(module, &header, sizeof(header)) == sizeof(header))
	    {
	      // todo get string table
	      dr_file_seek(module, header.e_shoff, DR_SEEK_SET);

	      while (nb_sect < header.e_shnum)
		{
		  if (dr_read_file(module, &section, header.e_shentsize) ==
		      INVALID_FILE)
		    return ;
		  // get from string table and if is .plt compute start/end value add this to the try and good bye
		  nb_sect++;
		}
	    }
	}
    }  
  
  dr_close_file(module);
}
