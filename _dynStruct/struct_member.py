
class StructMember:


    def __init__(self, offset, size, block):
        self.offset = offset
        self.size = size
        self.name = "offset_0x%x" % offset
        self.access = block.get_access_by_member(self)

    def __str__(self):
        s = "uint%d_t %s;\n" % (self.size * 8, self.name)
        return s
        
    def get_accesses(self):
        self.access = block.get_access_by_member(self)
