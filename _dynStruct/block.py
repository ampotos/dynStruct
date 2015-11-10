from . import Access

class Block:


    def __init__(self, block, l_access_w, l_access_r):
        self.struct = None
        self.start = block["start"]
        self.end = block["end"]
        self.size = block["size"]
        self.free = False
        if block["free"] == 1:
            self.free = True
        self.alloc_by_realloc = False
        if block["alloc_by_realloc"] == 1:
            self.alloc_by_realloc = True
        self.free_by_realloc = False
        if block["free_by_realloc"] == 1:
            self.free_by_realloc = True
        self.alloc_pc = block["alloc_pc"]
        self.alloc_func = block["alloc_func"]
        self.alloc_sym = block["alloc_sym"]
        self.alloc_module = block["alloc_module"]
        self.free_pc = block["free_pc"]
        self.free_func = block["free_func"]
        self.free_sym = block["free_sym"]
        self.free_module = block["free_module"]
        
        self.r_access = []
        self.w_access = []

        for access in filter(None, block["read_access"]):
            for orig in filter(None, access["details"]):
                self.r_access.append(Access(access, orig, self.start, self))
                l_access_r.append(self.r_access[-1])
            
        for access in filter(None, block["write_access"]):
                for orig in filter(None, access["details"]):
                    self.w_access.append(Access(access, orig, self.start, self))
                    l_access_w.append(self.w_access[-1])

    def get_access_by_offset(self, offset):
        ret = []
        for access in self.r_access:
            if access.is_offset(offset):
                ret.append(access)

        for access in self.w_access:
            if access.is_offset(offset):
                ret.append(access)
                
        return ret
                
    def get_access_by_member(self, member):
        ret = []
        for access in self.r_access:
            if access.in_member(member):
                ret.append(access)
                
        for access in self.w_access:
            if access.in_member(member):
                ret.append(access)

        return ret            
