import _dynStruct

class StructMember:


    def __init__(self, offset, size, t = None):
        self.offset = offset
        self.size = size
        if not t:
            self.t = "uint%d_t" % (self.size * 8)
        else:
            self.t = t
        if self.t == _dynStruct.ptr_func_str:
            self.web_t = self.t % ('ptr_func')
        else:
            self.web_t = self.t
        self.set_default_name()
        
        self.is_array = False
        self.number_unit = 0
        self.size_unit = 0

        self.is_padding = False

        self.is_struct = False
        self.is_array_struct = False
        self.is_sub_struct = False
        self.sub_struct = None
        
    def __str__(self):
        if self.is_array:
            s = self.print_array()
        elif self.is_struct:
            s = self.print_struct()
        elif self.is_array_struct:
            s = self.print_array_struct()
        elif self.t == _dynStruct.ptr_func_str:
            s = self.t % (self.name)
            s += ';\n'
        elif '//' in self.t:
            s = "%s %s; //%s\n" % (self.t.split('//')[0], self.name, self.t.split('//')[1])
        else:
            s = "%s %s;\n" % (self.t, self.name)

        return s


    def set_default_name(self):
        self.name = "offset_0x%x" % self.offset

    def to_sub_struct(self, offset):
        self.offset -= offset
        self.set_default_name()
        
    def same_type(self, other):

        if not self.is_array_struct or not other.is_array_struct:
            return self.t == other.t
        
        if self.size != other.size:
            return False

        return not False in [True if m1.same_type(m2) else False for (m1, m2) in
                             zip(self.sub_struct.members,
                                 other.sub_struct.members)]

    def same_or_padding(self, other):

        return self.t == other.t and\
            self.offset == other.offset and\
            self.size == other.size
        
    def print_struct(self):
        str_struct = "\n\t".join(str(self.sub_struct).split('\n')[:-1])[:-1]
        return str_struct + "%s;\n" % (self.name)

    def print_array_struct(self):
        str_struct = "\n\t".join(str(self.sub_struct).split('\n')[:-1])[:-1]
        return str_struct + "%s[%d];\n" % (self.name, self.number_unit)

    def print_array(self):
        if not '//' in self.t:
            ret = "%s %s[%d];\n" % (self.t, self.name, self.number_unit)
        else:
            ret = "%s %s[%d]; //%s\n" % (self.t.split('//')[0], self.name,
                                       self.number_unit, self.t.split('//')[1])
        return ret
    
    def set_array(self, nb_unit, size_unit):
        self.is_array = True
        self.number_unit = nb_unit
        self.size_unit = size_unit
        self.web_t = "array of %s" % (self.t)

    def set_struct(self, size, new_struct, name):
        self.is_struct = True
        self.is_sub_struct = True
        self.sub_struct = new_struct
        self.size = size
        self.name = name
        self.t = name
        self.web_t = "struct %s" % (name)
        
        new_struct.size = size
        new_struct.name = name
        
    def set_array_struct(self, nb_unit, size_unit, members_list, new_struct):
        self.is_array_struct = True
        self.number_unit = nb_unit
        self.sub_struct = new_struct
        self.is_sub_struct = True
        self.t = ""
        self.web_t = "array of %s" % (self.name)
        if len(members_list):
            self.size_unit = 0;
            for m in members_list:
                self.size_unit += m.size
        else:
            self.size_unit = size_unit
        self.size = self.size_unit * self.number_unit

        new_struct.size = self.size_unit
        new_struct.members = members_list
        new_struct.name = self.name

    def edit_struct_size(self, struct, new_size):
        if new_size != struct.size:
            if new_size < struct.size:
                old_struct_members = list(struct.members)
                for m in old_struct_members:
                    if m.offset + m.size > new_size:
                        struct.members.remove(m)
            struct.size = new_size
            # to have correct padding after the change of size
            struct.add_pad()
        
    def edit_array(self, forms, next_member, size_struct):
        try:
            size_unit = int(forms.size_unit)
            nb_unit = int(forms.nb_unit)
        except ValueError:
            raise ValueError("size_unit or nb_unit is not an integer")
        
        if size_unit <= 0 or nb_unit <= 0:
            raise ValueError("size_unit and nb_unit cannot be negative or Null")
    
        if size_unit * nb_unit + self.offset > size_struct:
            raise ValueError("Size is too big (there is not enough byte left in the struct)")
        
        if size_unit * nb_unit > self.size and \
           (not next_member or not next_member.is_padding or next_member.size + self.size < size_unit * nb_unit):
            raise ValueError("Size is too big (not enough padding between this member and the next one)")
        
        self.name = forms.name
        self.number_unit = nb_unit
        self.size_unit = size_unit
        self.size = size_unit * nb_unit
        self.t = forms.type
        self.web_t = "array of %s" % (self.t)

    def edit_struct(self, forms, next_member, size_struct):
        try:
            size = int(forms.size)
        except ValueError:
            return bottle.template("error", msg="Size is not an integer")
                
        if size <= 0:
            raise ValueError("Size cannot be negative or Null")
        
        if size + self.offset > size_struct:
            raise ValueError("Size is too big (there is not enough byte left in the struct)")
        
        if size > self.size and \
           (not next_member or not next_member.is_padding or next_member.size + self.size < size):
            raise ValueError("Size is too big (not enough padding between this member and the next one)")
        
        self.edit_struct_size(self.sub_struct, size)
        
        self.name = forms.name
        self.sub_struct.name = self.name
        self.size = size
        self.t = self.name
        self.web_t = "struct %s" % (self.t)
    
    def edit_array_struct(self, forms, next_member, size_struct):
         try:
             size_unit = int(forms.size_unit)
             nb_unit = int(forms.nb_unit)
         except ValueError:
             raise ValueError("size_unit or nb_unit is not an integer")
         
         if size_unit <= 0 or nb_unit <= 0:
             raise ValueError("size_unit and nb_unit cannot be negative or Null")
         
         if size_unit * nb_unit + self.offset > size_struct:
             raise ValueError("Size is too big (there is not enough byte left in the struct)")
         
         if size_unit * nb_unit > self.size and \
            (not next_member or not next_member.is_padding or next_member.size + self.size < size_unit * nb_unit):
             raise ValueError("Size is too big (not enough padding between this member and the next one)")
         
         self.name = forms.name
         self.number_unit = nb_unit
         self.size_unit = size_unit
         self.size = size_unit * nb_unit
         self.sub_struct.name = forms.type
         self.web_t = "array of %s" % (self.sub_struct.name)
         
         self.edit_struct_size(self.sub_struct, size_unit)

    def edit_simple(self, forms, next_member, size_struct):
        try:
                    size = int(forms.size)
        except ValueError:
            raise ValueError("Size is not an integer")
        
        if size <= 0:
            raise ValueError("Size cannot be negative or Null")
        
        if size + self.offset > size_struct:
            raise ValueError("Size is too big (there is not enough byte left in the struct)")
        
        if size > self.size and \
           (not next_member or not next_member.is_padding or next_member.size + self.size < size):
            raise ValueError("Size is too big (not enough padding between this member and the next one)")
        
        self.name = forms.name
        self.size = size
        self.t = forms.type
        self.web_t = self.t


    def edit(self, forms, next_member, size_struct):

        if self.is_array:
            self.edit_array(forms, next_member, size_struct)
        elif self.is_struct:
            self.edit_struct(forms, next_member, size_struct)
        elif self.is_array_struct:
            self.edit_array_struct(forms, next_member, size_struct)
        else:
            self.edit_simple(forms, next_member, size_struct)
