import _dynStruct
import json

def access_json_list(accesses, t):
    ret = []
    for access in accesses:
        if access.func_sym:
            instr_pc = '<span class="text-danger">%s%s%s%s%s' % (hex(access.pc),
                                                                 '</span>:<span class="text-success">',
                                                                 access.func_sym,
                                                                 '</span>+',
                                                                 hex(access.pc - access.func_pc))
        else:
            instr_pc = '<span class="text-danger">%s%s%s%s%s' % (hex(access.pc),
                                                                 '</span>:<span class="text-danger">',
                                                                 hex(access.func_pc),
                                                                 '</span>+',
                                                                 hex(access.pc - access.func_pc))
            
        func = '<span class="text-danger">%s%s%s' % (hex(access.func_pc),
                                                       '</span>@<span class="text-warning">',
                                                       access.func_module)
                                                       
        ret.append([t, hex(access.offset), hex(access.size), instr_pc, func])
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
