import _dynStruct
import json

def access_json_list(accesses, t, query, start_offset=0):
    ret = []
    for access in accesses:
        if not _dynStruct.filter_access(access, query, t):
            continue
        instr_pc = '<span class="text-danger">0x%x</span><strong>' % \
                   (access.pc & 0xffffffffffffffff)
        if access.func_sym:
            instr_pc += ':<span class="text-success">%s</span>' % \
                        (access.func_sym)
        else:
            instr_pc += ':<span class="text-danger">0x%x</span>' % \
                        (access.func_pc & 0xffffffffffffffff)
        if access.pc - access.func_pc > 0:
            instr_pc += '</strong>+0x%x' %(access.pc - access.func_pc)
        else:
            instr_pc += '</strong>%s' % (hex(access.pc - access.func_pc))
        instr_pc += '@<span class="text-warning">%s</span>' % \
                    (access.func_module)

        tmp = [t, hex(access.offset - start_offset), access.size, instr_pc,
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

# TODO : build json at load time for block and access
# for member and struct at each modif and just after recovery
# for block rebuild json when set or unset struct
def block_json_list(blocks, query):
    ret = []
    for block in blocks:
        if not _dynStruct.filter_block(block, query):
            continue
        tmp = ["0x%x" % (block.start & 0xffffffffffffffff),
               "%d" % (block.end - block.start)]

        alloc_pc = '<span class="text-danger">0x%x</span><strong>' % \
                   (block.alloc_pc & 0xffffffffffffffff)
        if block.alloc_sym:
            alloc_pc += ':<span class="text-success">%s</span>' % \
                        (block.alloc_sym)
        else:
            alloc_pc += ':<span class="text-danger">0x%x</span>' % \
                        (block.alloc_func & 0xffffffffffffffff)
        if block.alloc_pc - block.alloc_func > 0:
            alloc_pc += '</strong>+0x%x' %(block.alloc_pc - block.alloc_func)
        else:
            alloc_pc += '</strong>%s' % (hex(block.alloc_pc - block.alloc_func))
        alloc_pc += '@<span class="text-warning">%s</span>' % \
                    (block.alloc_module)
        tmp.append(alloc_pc)

        if block.free:
            free_pc = '<span class="text-danger">0x%x</span><strong>' % \
                      (block.free_pc & 0xffffffffffffffff)
            if block.free_sym:
                free_pc += ':<span class="text-success">%s</span>' % \
                           (block.free_sym)
            else:
                free_pc += ':<span class="text-danger">0x%x</span>' % \
                           (block.free_func & 0xffffffffffffffff)
            if block.free_pc - block.free_func > 0:
                free_pc += '</strong>+0x%x' %(block.free_pc - block.free_func)
            else:
                free_pc += '</strong>%s' % (hex(block.free_pc - block.free_func))
            free_pc += '@<span class="text-warning">%s</span>' % \
                           (block.free_module)
            tmp.append(free_pc)
        else:
            tmp.append("never free")
            
        tmp = ["<code>%s</code>" % (s) for s in tmp]
        if block.struct:
            tmp.append("<a href=/struct?id=%d>%s</a>" % (block.struct.id, block.struct.name))
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
               (id_struct, member.offset, member.name))
        tmp += ["%d" % (member.size),
               '<span class="text-warning">%s</span>' % (member.web_t)]
        ret.append(["<code>%s</code>" % (a) for a in tmp])
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

def struct_json():
    ret = []
    for struct in _dynStruct.l_struct:
        tmp = ['<span class="text-primary"><a href="/struct?id=%d">%s</a></span>' % (struct.id, struct.name),
               "%d" % (struct.size),
               "%d" % (struct.get_nb_members()),
               "%d" % (len(struct.blocks))]
        ret.append(["<code>%s</code>" % (a) for a in tmp])        
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

