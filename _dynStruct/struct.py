from .struct_member import StructMember

# list of classical string manipulation function
# use to detecte string
str_func = ["strlen", "strcpy", "strncpy", "strcmp", "strncmp", "strdup"]

# list of classical function which we ignore the access
# because there access are non relative to the struct
ignore_func = ["memset", "memcpy", "memcmp"]

# minimal size for a sub_array
min_size_array = 5

class Struct:


    def __init__(self, block):
        self.name = ""
        self.id = 0
        self.size = block.size
        self.blocks = []
        self.looks_array = False;
        self.size_array_unit = 0
        self.members = []

        self.recover(block)
        self.add_block(block)

    def __str__(self):
        s = "//total size : 0x%x\n" % self.size

        if len(self.members) == 1:
            s += str(self.members[0])
            return s
        
        s += "struct %s {\n" % self.name
        for member in self.members:
            s += "\t" + str(member)
        s += "};\n"
        return s
        
    def recover(self, block):
        actual_offset = 0

        while actual_offset < self.size:
            accesses = block.get_access_by_offset(actual_offset)
            if not accesses:
                actual_offset += 1
                continue

            self.filter_access(accesses)
            if self.has_str_access(accesses):
                self.change_to_str(block)
                break

            if len(accesses) == 0:
                actual_offset += 1
                continue
            
            size_member = self.get_best_size(block, actual_offset, accesses)
            self.members.append(StructMember(actual_offset, size_member, block))
            actual_offset += size_member

    def set_default_name(self):
        self.name = "struct_%d" % self.id            

    def clean_struct(self):
        self.add_pad()
        self.clean_array()
        # todo detection of tab of sub_struct
        self.clean_array_name()
        return
        
    def add_pad(self):
        old_offset = 0
        old_members = list(self.members)
        for member in old_members:
            if old_offset != member.offset:
                self.add_member_array(self.members.index(member),
                                      'pad_offset_0x%x' % old_offset,
                                      old_offset, member.offset - old_offset,
                                      'uint8_t', member.offset - old_offset, 1,
                                      None)

                old_offset = member.offset
            old_offset += member.size

        if old_offset < self.size:
            self.add_member_array(len(self.members),
                                  'pad_offset_0x%x' % old_offset, old_offset,
                                  self.size - old_offset, 'uint8_t',
                                  self.size - old_offset, 1, None)

    def clean_array(self):
        (index, index_end, nb_unit, size) = self.find_sub_array()
        while nb_unit:
            tmp_members = list(self.members)
            self.add_member_array(index,
                                  "array_0x%x" % self.members[index].offset,
                                  self.members[index].offset, size * nb_unit,
                                  "uint%d_t" % (size * 8), nb_unit, size, None)
            for member in tmp_members[index : index_end]:
                self.members.remove(member)
            (index, index_end, nb_unit, size) = self.find_sub_array()
            

    def clean_array_name(self):
        if len(self.members) == 1:
            self.members[0].name = "array_%d" % self.id
        
    def find_sub_array(self):
        for member in self.members:
            size = member.size
            ct = 0
            for m in self.members[self.members.index(member) :]:
                if (m.size == size and not m.is_array) or\
                   (m.is_array and m.size_unit == size and\
                    not m.name.startswith("pad_")):
                    if m.is_array:
                        ct += m.number_unit
                    else:
                        ct += 1
                else:
                    if ct >= min_size_array:
                        return (self.members.index(member),
                                self.members.index(m),
                                ct, size)
                    break
                
            if ct >= min_size_array:
                return (self.members.index(member),
                        len(self.members[self.members.index(member) :]),
                        ct, size)
                
        return (None, 0, 0, 0)
                        
    def add_member_array(self, index, name, offset, size, t,\
                         nb_unit, size_unit, block):
        new_member = StructMember(offset, size, block)
        new_member.name = name
        new_member.set_array(nb_unit, size_unit, t)
        self.members.insert(index, new_member)
            
    def get_best_size(self, block, offset, accesses):
        sizes = {}
        max_size = 0
        
        for access in accesses:
            if access.size > max_size:
                max_size = access.size
                
            if access.size in sizes:
                sizes[access.size] += 1
            else:
                sizes[access.size] = 1


        max_hit = 0
        for size in range(max_size + 1):
            if size in sizes and sizes[size] > max_hit:
                max_hit = sizes[size]

        return max([sz for sz in sizes if sizes[sz] == max_hit])
        
    def filter_access(self, accesses):
        for access in accesses:
            for func in ignore_func:
                if func.lower() in access.func_sym.lower():
                    accesses.remove(access)

    def has_str_access(self, accesses):
        for access in accesses:
            for func in str_func:
                if func.lower() in access.func_sym.lower():
                    return True
        return False
                    
    def change_to_str(self, block):
        self.members.clear()
        self.members = [StructMember(0, self.size, block)]
        self.members[0].set_array(self.size, 1, 'uint8_t')
        
    def block_is_struct(self, block):
        if block.size != self.size:
            return False

        actual_offset = 0
        while actual_offset < self.size:
            accesses = block.get_access_by_offset(actual_offset)
            if not accesses:
                actual_offset += 1
                continue

            self.filter_access(accesses)
            if self.has_str_access(accesses):
                if len(self.members) == 1 and self.members[0].is_array and\
                   self.members[0].size_unit == 1:
                    return True

            if len(accesses) == 0:
                actual_offset += 1
                continue
                
            size_member = self.get_best_size(block, actual_offset, accesses)
            if not self.has_member(actual_offset, size_member):
                return False
            
            actual_offset += size_member

        return True

    def has_member(self, offset, size):
        for member in self.members:
            if member.offset == offset and member.size == size:
                return True
        return False
    
    def add_block(self, block):
        self.blocks.append(block)
        block.struct = self
        self.maj_all_accesses()

    def remove_block(self, block):
        self.blocks.remove(block)
        block.struct = None
        self.maj_all_accesses()

    def look_like_array(self):
        if not len(self.members):
            return False

        size = self.members[0].size

        for member in self.members:
            if member.size != size:
                return False

        return True

    def maj_member_accesses(self, member):
        member.access.clear()
        for block in self.blocks:
            member.add_accesses_from_block(block)

    def maj_all_accesses(self):
        for member in self.members:
            self.maj_member_accesses(member)        
            
    def maj_member(self):
        self.maj_all_accesses()
        if self.look_like_array():
            self.looks_array = True
            self.size_array_unit = self.members[0].size
        else:
            self.looks_array = False

    @staticmethod
    def recover_all_struct(blocks, structs):        
        for block in blocks:
            if not block.w_access and not block.r_access:
                continue

            for struct in structs:
                if struct.block_is_struct(block):
                    struct.add_block(block)
                    break;

            if not block.struct:
                structs.append(Struct(block))

            if len(structs) == 0:
                continue

            if len(structs[-1].members) == 0:
                structs[-1].remove_block(block)
                structs.remove(structs[-1])
            else:
                structs[-1].id = len(structs)
                structs[-1].set_default_name()
                
    @staticmethod
    def clean_all_struct(structs):
        for struct in structs:
            struct.clean_struct()
