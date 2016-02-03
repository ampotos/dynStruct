

class StructMember:


    def __init__(self, offset, size, block):
        self.offset = offset
        self.size = size
        self.name = "offset_0x%x" % offset
        self.access = []
        self.t = "uint%d_t" % (self.size * 8)
        self.web_t = self.t
        
        self.is_array = False
        self.number_unit = 0
        self.size_unit = 0

        self.is_padding = False

        self.is_struct = False
        self.is_array_struct = False
        self.sub_struct = None

        self.add_accesses_from_block(block)
        
    def __str__(self):
        if self.is_array:
            s = self.print_array()
        elif self.is_array_struct:
            s = self.print_array_struct()
        else:
            s = "%s %s;\n" % (self.t, self.name)

        return s

    def same_type(self, other):

        if not self.is_array_struct and not other.is_array_struct:
            return self.t == other.t
        
        if self.size != other.size:
            return False
        
        return not False in [True if m1.same_type(m2) else False for (m1, m2) in
                             zip(self.sub_struct.members,
                                 other.sub_struct.members)]
        
    def print_array_struct(self):
        str_struct = "\n\t".join(str(self.sub_struct).split('\n')[1:-1])[:-1]
        return str_struct + "%s[%d];\n\n" % (self.name, self.number_unit)

    def print_array(self):
        return "%s %s[%d];\n" % (self.t, self.name, self.number_unit)
    
    def set_array(self, nb_unit, size_unit, t):
        self.is_array = True
        self.number_unit = nb_unit
        self.size_unit = size_unit
        self.t = t
        self.web_t = "array of %s" % (t)

    def set_array_struct(self, nb_unit, size_unit, members_list, new_struct):
        self.is_array_struct = True
        self.number_unit = nb_unit
        self.size_unit = size_unit
        new_struct.size = size_unit
        new_struct.members = members_list
        self.sub_struct = new_struct
        self.t = ""
        self.web_t = "array of %s" % (self.name)

        self.size = 0;
        for m in members_list:
            self.size += m.size
        self.size *= self.number_unit
        
        
    def add_accesses_from_block(self, block):
        if block:
            self.access += block.get_access_by_member(self)
