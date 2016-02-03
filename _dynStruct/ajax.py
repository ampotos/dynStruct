import _dynStruct
import json

def access_json_list(accesses, t):
    ret = []
    for access in accesses:
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

        tmp = [t, hex(access.offset), access.size, instr_pc,
                    '<a href=/block?id=%d>block_%d</a>' % \
                    (access.block.id_block, access.block.id_block)]
        ret.append(["<code>%s</code>" % (s) for s in tmp]) 
    return ret

def access_json_all():
    ret = access_json_list(_dynStruct.l_access_r, "read")
    ret += access_json_list(_dynStruct.l_access_w, "write")
    return ret

def access_json_from_block(id_block):
    ret = access_json_list(_dynStruct.l_block[id_block].r_access, "read")
    ret += access_json_list(_dynStruct.l_block[id_block].w_access, "write")
    return ret

def access_json_from_struct(id_struct):
    ret = []
    for block in _dynStruct.l_struct[id_struct]:
        ret += access_json_from_block(block.id_block)

    return ret

def access_json(id_block, id_struct):
    if id_block != None:
        ret = access_json_from_block(id_block)
    elif id_struct != None:
        ret = access_json_from_struct(id_struct)
    else:
        ret = access_json_all()
    return json.dumps({"draw" : 1,
                      "recordsTotal" : len(ret),
                      "recordsFiltered": len(ret),
                      "data" : ret})

def block_json_list(blocks):
    ret = []
    for block in blocks:
        tmp = ["0x%s" % (block.start & 0xffffffffffffffff),
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
    return ret
        
def block_json_from_struct(id_struct):
    for struct in _dynStruct.l_struct:
        if id_struct == struct.id:
            return block_json_list(struct.blocks)
    return []

def block_json(id_struct):
    if id_struct != None:
        ret = block_json_from_struct(id_struct)
    else:
        ret = block_json_list(_dynStruct.l_block)
    return json.dumps({"draw" : 1,
                       "recordsTotal" : len(ret),
                       "recordsFiltered": len(ret),
                       "data" : ret})

def member_json(struct, old_member):
    ret = []
    for member in struct.members:
        tmp = ["0x%x" % (member.offset)]
        if member.is_padding:
            tmp.append("padding")
        else:
            tmp.append('<span class="text-primary"><a href="/member?id_struct=%d&id_member=%s">%s</a></span>' %\
               (struct.id, member.offset, member.name))
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

