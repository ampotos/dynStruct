from .struct_member import StructMember
import _dynStruct

# list of classical string manipulation function
# use to detecte string
str_func = ["strlen", "strcpy", "strncpy", "strcmp", "strncmp", "strdup"]

# list of classical function which we ignore the access
# because there access are non relative to the struct
ignore_func = ["memset", "memcpy", "memcmp"]

# minimal size for a sub_array
min_size_array = 5

# size for basic type
# use to remove struct with only 1 member of one of these sizes
# (or array of unit size of one of these sizes)
base_size = [1, 2, 4, 8, 16]

class Struct:


    def __init__(self, block):
        self.name = ""
        self.id = 0
        self.blocks = []
        self.looks_array = False;
        self.size_array_unit = 0
        self.members = []

        if block:
            self.size = block.size
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

    def get_nb_members(self):
        nb_members = 0
        for member in self.members:
            if not member.is_padding:
                nb_members += 1
        return nb_members
    
    def get_member(self, offset):
        for member in self.members:
            if member.offset == offset:
                return member
        return None

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
        self.clean_array_struct()
        # we recall add_pad because clean_array_struct can remove members
        # this mean if a struct have a pad of 10 and 2 in this pad is in a
        # struct array, all the pad is remove
        self.add_pad()
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
                                      None, True)

                old_offset = member.offset
            old_offset += member.size

        if old_offset < self.size:
            self.add_member_array(len(self.members),
                                  'pad_offset_0x%x' % old_offset, old_offset,
                                  self.size - old_offset, 'uint8_t',
                                  self.size - old_offset, 1, None, True)

    def clean_array(self):
        (index, index_end, nb_unit, size) = self.find_sub_array()
        while nb_unit:
            tmp_members = list(self.members)
            self.add_member_array(index,
                                  "array_0x%x" % self.members[index].offset,
                                  self.members[index].offset, size * nb_unit,
                                  "uint%d_t" % (size * 8), nb_unit, size, None, False)
            for member in tmp_members[index : index_end]:
                self.members.remove(member)
            (index, index_end, nb_unit, size) = self.find_sub_array()


    def clean_array_struct(self):
        (index_start, index_end) = self.get_struct_pattern(0)

        while index_start != index_end:
            tmp_members = list(self.members)
            nb_unit = self.get_nb_pattern(index_start, index_end)
            self.add_member_array_struct(index_start, index_end,
                                         "struct_array_0x%x" %
                                         self.members[index_start].offset,
                                         self.members[index_start].offset,
                                         nb_unit, index_end - index_start,
                                         None)

            for member in tmp_members[index_start :
                                      index_start + (index_end - index_start) * nb_unit]:
                member.to_sub_struct(self.members[index_start].offset)
                self.members.remove(member)
            (index_start, index_end) = self.get_struct_pattern(index_start + 1)
            
        (index_start, index_end) = self.get_struct_pattern(0)
        if index_start != index_end:
            self.clean_array_struct()
            
    def clean_array_name(self):
        if len(self.members) == 1:
            self.members[0].name = "array_%d" % self.id

    def get_struct_pattern(self, index):
        for start_idx in range(index, len(self.members)):
            half_size = int(len(self.members[start_idx + 1:]) / 2)
            for size in range(2, half_size):
                tmp_idx = start_idx + size
                if False not in [True if m1.same_type(m2) else False for (m1, m2) in
                                 zip(self.members[start_idx : start_idx + size],
                                     self.members[tmp_idx : tmp_idx + size])]:
                    return (start_idx, start_idx + size)
        return (0, 0)
            
    def get_nb_pattern(self, index_start, index_end):
        size = index_end - index_start
        nb_unit = 1
        while nb_unit * size <= len(self.members[index_start:]) and\
              not False in [True if m1.same_type(m2) else False for (m1, m2) in
                            zip(self.members[index_start : index_start + size],
                                self.members[index_start + nb_unit * size :\
                                             index_start + nb_unit * size + size])]:

            nb_unit += 1
        return nb_unit
            
    def find_sub_array(self):
        for member in self.members:
            size = member.size
            ct = 0
            for m in self.members[self.members.index(member) :]:
                if (m.size == size and not m.is_array) or\
                   (m.is_array and m.size_unit == size and not m.is_padding):
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
                        self.members.index(self.members[-1]) + 1,
                        ct, size)
                
        return (None, 0, 0, 0)
                        
    def add_member_array(self, index, name, offset, size, t,
                         nb_unit, size_unit, block, padding):
        new_member = StructMember(offset, size, block)
        new_member.name = name
        new_member.set_array(nb_unit, size_unit, t)
        new_member.is_padding = padding
        if padding:
            new_member.web_t = "padding"
        self.members.insert(index, new_member)

    def add_member_array_struct(self, index, index_end, name, offset,
                                nb_unit, size_unit, block):

        new_member = StructMember(offset, 0, block)
        new_member.name = name
        new_member.set_array_struct(nb_unit, size_unit,
                                    self.members[index : index_end],
                                    Struct(None))
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

    def remove_block(self, block):
        self.blocks.remove(block)
        block.struct = None

    def remove_all_block(self):
        for block in self.blocks:
            self.remove_block(block)

    def look_like_array(self):
        if not len(self.members):
            return False

        size = self.members[0].size

        for member in self.members:
            if member.size != size:
                return False

        return True

    def maj_member(self):
        if self.look_like_array():
            self.looks_array = True
            self.size_array_unit = self.members[0].size
        else:
            self.looks_array = False

    def not_a_struct(self):

        if not self.members[0].is_array_struct and\
           not self.members[0].is_struct and\
           ((self.members[0].is_array and\
             self.members[0].size_unit in base_size) or\
            (not self.members[0].is_array and\
             self.members[0].size in base_size)):
            if len(self.members) == 1:
                return True
            else:
                for member in self.members[1:]:
                    if not member.is_padding:
                        return False
                return True
                    
        return False

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
                structs[-1].remove_all_block()
                structs.remove(structs[-1])
            else:
                structs[-1].id = len(structs)
                structs[-1].set_default_name()
                
    @staticmethod
    def clean_all_struct(structs):
        list_structs = list(structs)
        for struct in list_structs:
            struct.clean_struct()
            if struct.not_a_struct():
                struct.remove_all_block()
                structs.remove(struct)

    @staticmethod
    def get_by_id(id_struct, struct=None):
        if id_struct == None:
            return None
        if not '.' in id_struct and struct == None:
            for struct in _dynStruct.l_struct:
                if struct.id == int(id_struct):
                    return struct
            return None

        id_struct = id_struct.split('.')
        next_struct = '.'.join(id_struct[1:]) if len(id_struct) > 1 else None
        id_struct = id_struct[0]
        if not struct:
            struct = Struct.get_by_id(id_struct)
            return Struct.get_by_id(next_struct, struct)
        else:
            struct = struct.get_member(int(id_struct)).sub_struct
            if next_struct:
                return Struct.get_by_id(next_struct, struct)
            return struct

        return None        
                
    @staticmethod
    def get_member_by_id(id_struct, id_member):
        struct = Struct.get_by_id(id_struct)
        for member in struct.members:
            if member.offset == int(id_member):
                return member
        return None

    @staticmethod
    def make_member_name(id_struct, id_member):
        name = []
        id_struct_split = id_struct.split('.')
        for n in range(len(id_struct_split)):
            name.append(Struct.get_by_id('.'.join(id_struct_split[:n+1])).name)
        name.append(Struct.get_member_by_id(id_struct, id_member).name)
        return '.'.join(name)

    @staticmethod
    def get_member_access(id_struct):
        id_struct = id_struct.split('.')
        base_struct = _dynStruct.Struct.get_by_id(id_struct[0])

        tmp_id = id_struct[0]
        offsets = [0]
        for id_member in id_struct[1:]:
            member = _dynStruct.Struct.get_member_by_id(tmp_id, id_member)
            tmp_id += '.' + id_member

            offsets = [off + member.offset for off in offsets]
            if not member.is_array_struct:
                continue
            tmp_offsets = []
            for unit in range(member.number_unit):
                tmp_offsets += [off + unit * member.size_unit for off in offsets]
            offsets = tmp_offsets
            
        start_offset = sum([int(offset) for offset in id_struct[1:]])

        member_size = _dynStruct.Struct.get_member_by_id('.'.join(id_struct[:-1]),
                                                         int(id_struct[-1])).size
        r_access = []
        w_access = []
        for offset in offsets:
            for block in base_struct.blocks:
                (tmp_r_access, tmp_w_access) = block.get_access_by_range(offset, offset + member_size)
            r_access.append({"start" : offset, "access" : tmp_r_access})
            w_access.append({"start" : offset, "access" : tmp_w_access})

        return (r_access, w_access)
