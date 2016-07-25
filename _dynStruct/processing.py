import _dynStruct

def paging(start, size, l):
    return l[start:start + size]

def block_contain(addr, block):
    if ("0x%x" % block.start).startswith(addr):
        return True
    
    try:
        if addr.startswith('0x'):
            value = int(addr, 16)
        else:
            value = int(addr)
    except:
        return False

    # if addr is in block's address space
    if value >= block.start and value < block.end:
        return True

    return False

def size_filter(size, size_block):
    return ("%d" % size_block).startswith(size)

def offset_filter(offset, offset_block, query):
    # fix offset if we are in member view
    id_member = query.id_member
    if id_member and id_member != "None":
        for off in id_member.split('.')[1:]:
            offset_block -= int(off)
    return (hex(offset_block).startswith(offset))

def agent_filter(agent_str, addr, module, func_sym, func, offset):
    if func_sym:
        agent = "0x%x:%s" % (addr, func_sym)
    else:
        agent = "0x%x:0x%x" % (addr, func)

    if offset > 0:
        agent += "+0x%x@%s" %(offset, module)
    else:
        agent += "%s@%s" %(hex(offset), module)

    return agent_str in agent
    
def filter_block(block, query):
    if query["columns[0][search][value]"]:
        if not block_contain(query["columns[0][search][value]"], block):
            return False
        
    if query["columns[1][search][value]"]:
        if not size_filter(query["columns[1][search][value]"], block.size):
            return False

    if query["columns[2][search][value]"]:
        if not agent_filter(query["columns[2][search][value]"], block.alloc_pc,
                            block.alloc_module, block.alloc_sym, block.alloc_func,
                            block.alloc_pc - block.alloc_func):
            return False

    if not block.free:
        if not "never free".startswith(query["columns[3][search][value]"]):
            return False
    elif query["columns[3][search][value]"]:
        if not agent_filter(query["columns[3][search][value]"], block.free_pc,
                            block.free_module, block.free_sym, block.free_func,
                            block.free_pc - block.free_func):
            return False

    return True

def filter_access(access, query, t):
    if query["columns[0][search][value]"]:
        if not t.startswith(query["columns[0][search][value]"]):
            return False
        
    if query["columns[1][search][value]"]:
        if not offset_filter(query["columns[1][search][value]"], access.offset, query):
            return False

    if query["columns[2][search][value]"]:
        if not size_filter(query["columns[2][search][value]"], access.size):
            return False

    if query["columns[3][search][value]"]:
        if query["columns[3][search][value]"] != str(access.nb_access):
            return False

    if query["columns[4][search][value]"]:
        if not agent_filter(query["columns[4][search][value]"], access.pc,
                            access.func_module, access.func_sym, access.func_pc,
                            access.pc - access.func_pc):
            return False

    if query["columns[5][search][value]"]:
        if not query["columns[5][search][value]"] in access.instr_search:
            return False

    if query["columns[6][search][value]"]:
        if not query["columns[6][search][value]"] in access.ctx_instr_search:
            return False

    return True

# start size malloc_caller free_caller struct detailed_view 
def sorting_block(l, column, order):
    order = True if order == "asc" else False
    getter = {"0": lambda item: int(item[0].split('>')[1].split('<')[0], 16),
              "1": lambda item: int(item[1].split('>')[1].split('<')[0]),
              "2": lambda item: int(item[2].split('>')[2].split('<')[0], 16),
              "3": lambda item: int(item[3].split('>')[2].split('<')[0], 16) if "never free" not in item[3] else 0,
              "4": lambda item: int(item[4].split('=')[2].split('>')[0])  if "None" not in item[4] else 0,
              "5": lambda item: int(item[5].split('_')[1].split('<')[0])
              }
    l.sort(key=getter[column] ,reverse=order)
    return l

# access offset size agent block_id
def sorting_access(l, column, order):
    order = True if order == "asc" else False
    getter = {"0": lambda item: item[0],
              "1": lambda item: int(item[1].split('>')[1].split('<')[0], 16),
              "2": lambda item: int(item[2].split('>')[1].split('<')[0]),
              "3": lambda item: item[3],
              "4": lambda item: int(item[4].split('>')[2].split('<')[0], 16),
              "5": lambda item: item[5],
              "6": lambda item: item[6],
              "7": lambda item: int(item[7].split('_')[1].split('<')[0])
              }
    l.sort(key=getter[column] ,reverse=order)
    return l
