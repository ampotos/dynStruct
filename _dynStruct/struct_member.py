
class StructMember:


    def __init__(self, offset, size, block):
        self.offset = offset
        self.size = size
        self.name = "offset_0x%x" % offset
        self.access = []
        self.type = "uint%d_t" % (self.size * 8)

        self.is_array = False
        self.number_unit = 0
        self.size_unit = 0

        self.is_padding = False
        
        self.is_struct = False
        self.sub_struct = None

        self.add_accesses_from_block(block)
        
    def __str__(self):
        if self.is_array:
            s = self.print_array()
        elif self.is_struct:
            s = self.print_struct()
        else:
            s = "%s %s;\n" % (self.type, self.name)

        return s

    def print_struct(self):
        return

    def print_array(self):
        return "%s %s[%d];\n" % (self.type, self.name, self.number_unit)
    
    def set_array(self, nb_unit, size_unit, t):
        self.is_array = True
        self.number_unit = nb_unit
        self.size_unit = size_unit
        self.type = t

    def set_sub_struct(self, struct):
        self.is_struct = True
        self.sub_struct = struct
        
    def add_accesses_from_block(self, block):
        if block:
            self.access += block.get_access_by_member(self)
