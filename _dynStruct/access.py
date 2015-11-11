
class Access:


    def __init__(self, access, orig, addr_start, block):
        self.block = block

        self.offset = access["offset"]
        self.addr = addr_start + self.offset
        self.size = orig["size_access"]
        self.nb_hit = orig["nb_access"]
        self.pc = orig["pc"]
        self.func_addr = orig["func_pc"]
        self.func_sym = orig["func_sym"]
        self.func_module = orig["func_module"]

    def is_offset(self, offset):
        return self.offset == offset


    def in_member(self, member):
        if self.is_offset(member.offset):
            return True

        if self.offset >= member.offset and\
           self.offset < member.offset + member.size:
            return True

        return False
