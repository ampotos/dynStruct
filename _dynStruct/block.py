from . import Access

class Block:


    def __init__(self, block, l_access_w, l_access_r):
        self.struct = None

        cast_attr = {"free_by_realloc" : bool,
                     "alloc_by_realloc" : bool,
                     "free" : bool}
        json_attrib = ["start", "end", "size", "free", "alloc_by_realloc",
                       "free_by_realloc", "alloc_pc", "alloc_func", "alloc_sym",
                       "alloc_module", "free_pc", "free_func", "free_sym",
                       "free_module"]
        
        for k in json_attrib:
            setattr(self, k, cast_attr.get(k, block[k].__class__)(block[k]))
            
        self.r_access = []
        self.w_access = []

        for access in filter(None, block["read_access"]):
            for orig in filter(None, access["details"]):
                self.r_access.append(Access(access["offset"], orig, self.start, self))
                l_access_r.append(self.r_access[-1])
            
        for access in filter(None, block["write_access"]):
                for orig in filter(None, access["details"]):
                    self.w_access.append(Access(access["offset"], orig, self.start, self))
                    l_access_w.append(self.w_access[-1])

    def get_access_by_offset(self, offset):
        ret = []
        for access in self.r_access + self.w_access:
            if access.is_offset(offset):
                ret.append(access)

        return ret
                
    def get_access_by_member(self, member):
        ret = []
        for access in self.r_access + self.w_access:
            if access.in_member(member):
                ret.append(access)

        return ret            
