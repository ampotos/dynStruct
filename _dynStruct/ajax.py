import _dynStruct
import json
import html

def make_pc_display(pc, sym, func_pc, func_module):
    display = '<span class="text-danger">0x%x</span><strong>' % \
              (pc & 0xffffffffffffffff)
    if sym:
        display += ':<span class="text-success">%s</span>' % \
                   (html.escape(sym))
    else:
        display += ':<span class="text-danger">0x%x</span>' % \
                   (func_pc & 0xffffffffffffffff)
    if pc - func_pc > 0:
        display += '</strong>+0x%x' % (pc - func_pc)
    else:
        display += '</strong>%s' % (hex(pc - func_pc))
    display += '@<span class="text-warning">%s</span>' % \
               (html.escape(func_module))

    return display

def make_instr_display(instr):
    return '<span class="text-success"><strong>%s</strong></span>\
    <span class="text-info">%s</span>' % (instr.mnemonic, instr.op_str)


def access_json_list(accesses, t, query, start_offset=0):
    ret = []
    for access in accesses:
        if not _dynStruct.filter_access(access, query, t):
            continue

        instr_pc = make_pc_display(access.pc, access.func_sym, access.func_pc,
                                   access.func_module)

        tmp = [t, hex(access.offset - start_offset), access.size, instr_pc,
               access.instr_display, access.ctx_instr_display,
               '<a href=/block?id=%d>block_%d</a>' % \
               (access.block.id_block, access.block.id_block)]
        ret.append(["<code>%s</code>" % (s) for s in tmp]) 
    return (len(accesses), ret)

def access_json_all(query):
    (total, ret) = access_json_list(_dynStruct.l_access_r, "read", query)
    (tmp_total, tmp_ret) = access_json_list(_dynStruct.l_access_w, "write", query)
    return (total + tmp_total, ret + tmp_ret)

def access_json_from_block(id_block, query):
    (total, ret) = access_json_list(_dynStruct.l_block[id_block].r_access, "read", query)
    (tmp_total, tmp_ret) = access_json_list(_dynStruct.l_block[id_block].w_access, "write", query)
    return (total + tmp_total, ret + tmp_ret)

def access_json_from_struct(id_member, query):
    (r_access, w_access) = _dynStruct.Struct.get_member_access(id_member)
    total = 0
    ret = []
    for (tmp_r_access, tmp_w_access) in zip(r_access, w_access):
        (tmp_total, tmp_ret) = access_json_list(tmp_r_access["access"], "read", query, tmp_r_access["start"])
        total += tmp_total
        ret += tmp_ret
        (tmp_total, tmp_ret) = access_json_list(tmp_w_access["access"], "write", query, tmp_w_access["start"])
        total += tmp_total
        ret += tmp_ret
    return (total, ret)

def access_json(id_block, id_member, query):
    if id_block != None:
        (total, ret) = access_json_from_block(id_block, query)
    elif id_member != None:
        (total, ret) = access_json_from_struct(id_member, query)
    else:
        (total, ret) = access_json_all(query)

    total_filtered = len(ret)
    ret = _dynStruct.sorting_access(ret, query['order[0][column]'], query['order[0][dir]'])
    ret = _dynStruct.paging(int(query["start"]), int(query["length"]), ret)
    return json.dumps({"draw" : query["draw"],
                       "recordsTotal" : total,
                       "recordsFiltered": total_filtered,
                       "data" : ret})


def block_json_list(blocks, query):
    ret = []
    for block in blocks:
        if query and not _dynStruct.filter_block(block, query):
            continue
        tmp = ["0x%x" % (block.start & 0xffffffffffffffff),
               "%d" % (block.end - block.start)]

        alloc_pc = make_pc_display(block.alloc_pc, block.alloc_sym,
                                   block.alloc_func, block.alloc_module)
        tmp.append(alloc_pc)

        if block.free or block.free_by_realloc:
            free_pc = make_pc_display(block.free_pc, block.free_sym,
                                       block.free_func, block.free_module)
            tmp.append(free_pc)
        else:
            tmp.append("never free")
            
        tmp = ["<code>%s</code>" % (s) for s in tmp]
        if block.struct:
            tmp.append("<a href=/struct?id=%d>%s</a>" % (block.struct.id, html.escape(block.struct.name)))
        else:
            tmp.append("None")
        tmp.append("<a href=/block?id=%d>block_%d</a>" % (block.id_block, block.id_block))
        ret.append(tmp)
    return (len(blocks), ret)
        
def block_json_from_struct(id_struct, query):
    for struct in _dynStruct.l_struct:
        if int(id_struct) == struct.id:
            return block_json_list(struct.blocks, query)
    return (0, [])

def block_json(id_struct, query):
    if id_struct != None:
        (total, ret) = block_json_from_struct(id_struct, query)
    else:
        (total, ret) = block_json_list(_dynStruct.l_block, query)

    total_filtered = len(ret)
    ret = _dynStruct.sorting_block(ret, query['order[0][column]'], query['order[0][dir]'])
    ret = _dynStruct.paging(int(query["start"]), int(query["length"]), ret)
    return json.dumps({"draw" : query["draw"],
                       "recordsTotal" : total,
                       "recordsFiltered": total_filtered,
                       "data" : ret})

def member_json(struct, id_struct):
    ret = []
    for member in struct.members:
        tmp = ["0x%x" % (member.offset)]
        if member.is_padding:
            tmp.append("")
        else:
            tmp.append('<span class="text-primary"><a href="/member?id_struct=%s&id_member=%s">%s</a></span>' %\
               (id_struct, member.offset, html.escape(member.name)))
        tmp += ["%d" % (member.size),
               '<span class="text-warning">%s</span>' % (html.escape(member.web_t))]
        if member.is_padding:
            tmp.append("<a href=/member_create?id_struct=%s&id_member=%d>Add member</a>" % (id_struct, member.offset))
        else:
            tmp.append("<a href=/member_edit?id_struct=%s&id_member=%s>Edit member</a>" % (id_struct, member.offset))
        ret.append(["<code>%s</code>" % (a) for a in tmp])
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

def struct_json():
    ret = []
    for struct in _dynStruct.l_struct:
        tmp = ['<span class="text-primary"><a href="/struct?id=%d">%s</a></span>' % (struct.id, html.escape(struct.name)),
               "%d" % (struct.size),
               "%d" % (struct.get_nb_members()),
               "%d" % (len(struct.blocks))]
        ret.append(["<code>%s</code>" % (a) for a in tmp])        
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

def struct_select_json(id_block):
    ret = []
    if id_block or id_block == 0:
        size = _dynStruct.l_block[id_block].size
        for struct in filter(lambda x: x.size == size,  _dynStruct.l_struct):
            tmp = ['<span class="text-primary"><a href="/struct?id=%d">%s</a></span>' % (struct.id, html.escape(struct.name)),
                   "%d" % (struct.get_nb_members()),
                   "%d" % (len(struct.blocks)),
                   '<a href="/do_add_to_struct?id_block=%d&id_struct=%d">link with this structure</a>' % (id_block, struct.id)]
            ret.append(["<code>%s</code>" % a for a in tmp])
        
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

def struct_instances_json(struct, instance):
    ret = []
    if instance:
        (useless, ret) = block_json_list(struct.blocks, None)
        for block in ret:
            block.pop(4)
            block.append(struct.blocks[ret.index(block)].id_block)
    else:
        potential_blocks = []
        for block in _dynStruct.l_block:
            if not block.struct and block.size == struct.size:
                potential_blocks.append(block)

        (useless, ret) = block_json_list(potential_blocks, None)
        for block in ret:
            block.pop(4)
            block.append(potential_blocks[ret.index(block)].id_block)

    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})
