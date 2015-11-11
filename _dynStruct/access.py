
class Access:


    def __init__(self, access, orig, addr_start, block):
        self.block = block
        self.offset = access
        self.addr = addr_start + self.offset
        self.size = orig["size_access"]
        
        json_attrib = ["nb_access", "pc", "func_pc",
                       "func_sym", "func_module"]
        
        for k in json_attrib:
            setattr(self, k, (orig[k]))
        
    def is_offset(self, offset):
        return self.offset == offset


    def in_member(self, member):
        if self.is_offset(member.offset):
            return True

        if self.offset >= member.offset and\
           self.offset < member.offset + member.size:
            return True

        return False
