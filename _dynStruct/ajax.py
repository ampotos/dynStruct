import _dynStruct
import json

def access_json_list(accesses, t):
    ret = []
    for access in accesses:
        instr_pc = '<code><span class="text-danger">0x%x</span>' % \
                   (access.pc & 0xffffffffffffffff)
        instr_pc += '@<span class="text-warning">%s</span>' % \
                    (access.func_module)
        if access.func_sym:
            instr_pc += ':<span class="text-success">%s</span>' % \
                        (access.func_sym)
        else:
            instr_pc += ':<span class="text-danger">0x%x</span>' % \
                        (access.func_pc & 0xffffffffffffffff)
        instr_pc += '+0x%x</code>' %((access.pc - access.func_pc) & 0xffffffffffffffff)
        
        ret.append([t, hex(access.offset), access.size, instr_pc,
                    '<a href=/block?id=%d>block_%d</a>' % \
                    (access.block.id_block, access.block.id_block)])
    return ret

def access_json_all():
    ret = access_json_list(_dynStruct.l_access_r, "read")
    ret += access_json_list(_dynStruct.l_access_w,"write")
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
    if id_block:
        ret = access_json_from_block(id_block)
    elif id_struct:
        ret = access_json_from_struct(id_struct)
    else:
        ret = access_json_all()
    return json.dumps({"draw" : 1,
                      "recordsTotal" : len(ret),
                      "recordsFiltered": len(ret),
                      "data" : ret})
