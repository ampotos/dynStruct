from .struct_member import StructMember
import _dynStruct
import pyprind
import copy

# list of classical function which we ignore the access
# because there access are non relative to the struct
ignore_func = ["memset", "memcpy", "memcmp", "mempcpy"]

# list of classical-# use to detecte string
str_func = ["strlen", "strcpy", "strncpy", "strcmp", "strncmp", "strdup", "strcat"]

# minimal size for a sub_array
min_size_array = 5

class Struct:


    def __init__(self, block, is_sub=False):
        self.name = ""
        self.id = 0
        self.blocks = []
        self.looks_array = False;
        self.size_array_unit = 0
        self.members = []
        self.is_sub_struct = is_sub
        self.size = 0
        
        if block:
            self.size = block.size
            self.recover(block)
            self.add_block(block)

    def __str__(self):
        s = ""
        if not self.is_sub_struct:
            s += "//total size : 0x%x\n" % self.size

        if len(self.members) == 1 and not self.is_sub_struct:
            s += str(self.members[0])
            return s

        if not self.is_sub_struct:
            s += "struct %s \n{" % self.name
        else:
            s += "struct {\n"
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
            if member.offset == offset or \
               (member.offset < offset and member.offset + member.size > offset):
                return member
        return None

    def insert_member(self, new_member):
        # call only by merge, so only on offset without member yet
        for member in self.members:
            if member.offset > new_member.offset:
                self.members.insert(self.members.index(member), new_member)
                return

        # new member go at the end fo the struct
        self.members.append(new_member)

    def recover(self, block):
        actual_offset = 0

        if not _dynStruct.disasm:
            _dynStruct.create_disasm()

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
            
            size_member = self.get_best_size(accesses)
            t = self.get_type(accesses, size_member)
            self.members.append(StructMember(actual_offset, size_member, t))
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
        self.set_array_name()
        return
        
    def add_pad(self):
        old_offset = 0
        old_members = list(self.members)
        for member in old_members:
            if old_offset < member.offset:
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
        (index, index_end, nb_unit) = self.find_sub_array()
        while nb_unit:
            tmp_members = list(self.members)
            size = self.members[index].size
            self.add_member_array(index,
                                  "array_0x%x" % self.members[index].offset,
                                  self.members[index].offset, size * nb_unit,
                                  self.members[index].t, nb_unit, size, None, False)
            for member in tmp_members[index : index_end]:
                self.members.remove(member)
            (index, index_end, nb_unit) = self.find_sub_array()

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
            
    def set_array_name(self):
        if len(self.members) == 1:
            self.members[0].name = "array_%d" % self.id

    def get_struct_pattern(self, index):
        for start_idx in range(index, len(self.members)):
            half_size = int(len(self.members[start_idx + 1:]) / 2)
            for size in range(2, half_size):
                tmp_idx = start_idx + size
                if size < min_size_array and len(set([m.t for m in\
                                                     self.members[start_idx :\
                                                                  start_idx +\
                                                                  size]])) == 1:
                    continue
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
            t = member.t
            size = member.size
            ct = 0
            for m in self.members[self.members.index(member) :]:
                if (m.size == size and m.t == t and not m.is_array) or\
                   (m.is_array and m.size_unit == size and\
                    m.t == t and not m.is_padding):
                    if m.is_array:
                        ct += m.number_unit
                    else:
                        ct += 1
                else:
                    if ct >= min_size_array:
                        return (self.members.index(member),
                                self.members.index(m),
                                ct)
                    break
                
            if ct >= min_size_array:
                return (self.members.index(member),
                        self.members.index(self.members[-1]) + 1,
                        ct)
                
        return (None, 0, 0)
                        
    def add_member_array(self, index, name, offset, size, t,
                         nb_unit, size_unit, block, padding):
        new_member = StructMember(offset, size, t=t)
        new_member.name = name
        new_member.set_array(nb_unit, size_unit)
        new_member.is_padding = padding
        if padding:
            new_member.web_t = "padding"
        self.members.insert(index, new_member)

    def add_member_array_struct(self, index, index_end, name, offset,
                                nb_unit, size_unit, block):

        new_member = StructMember(offset, 0)
        new_member.name = name
        new_member.set_array_struct(nb_unit, size_unit,
                                    self.members[index : index_end],
                                    Struct(None, is_sub=True))
        self.members.insert(index, new_member)

    def create_simple(self, forms, offset, end_pad):
        size = forms.size
        try:
            size = int(size)
        except ValueError:
            raise ValueError("Size is not an integer")

        if size <= 0:
            raise ValueError("Size have to be positive")

        if offset + size > end_pad:
            raise ValueError("The new member does not entirely in the padding (offset + size > end_padding")
        new_member = StructMember(offset, size)
        new_member.name = forms.name
        new_member.t = forms.type
        new_member.web_t = new_member.t

        return new_member

    def create_array(self, forms, offset, end_pad):
        size_unit = forms.size_unit
        nb_unit = forms.nb_unit
        try:
            size_unit = int(size_unit)
            nb_unit = int(nb_unit)
        except ValueError:
            raise ValueError("Size_unit or nb_unit is not an integer")

        if size_unit <= 0 or nb_unit <= 0:
            raise ValueError("Size_unit and nb_unit have to be positive")

        if offset + size_unit * nb_unit > end_pad:
            raise ValueError("The new member does not entirely in the padding (offset + size_unit * nb_unit > end_padding")

        new_member = StructMember(offset, nb_unit * size_unit)
        new_member.name = forms.name
        new_member.set_array(nb_unit, size_unit, forms.type)

        return new_member

    def create_struct(self, forms, offset, end_pad):
        size = forms.size
        try:
            size = int(size)
        except ValueError:
            raise ValueError("Size is not an integer")

        if size <= 0:
            raise ValueError("Size have to be positive")

        if offset + size > end_pad:
            raise ValueError("The new member does not entirely in the padding (offset + size > end_padding")

        new_member = StructMember(offset, size)
        sub_struct = Struct(None, is_sub=True)
        new_member.set_struct(size, sub_struct, forms.name)
        sub_struct.add_pad()
        
        return new_member


    def create_array_struct(self, forms, offset, end_pad):
        size_unit = forms.size_unit
        nb_unit = forms.nb_unit
        try:
            size_unit = int(size_unit)
            nb_unit = int(nb_unit)
        except ValueError:
            raise ValueError("Size_unit or nb_unit is not an integer")

        if size_unit <= 0 or nb_unit <= 0:
            raise ValueError("Size_unit and nb_unit have to be positive")

        if offset + size_unit * nb_unit > end_pad:
            raise ValueError("The new member does not entirely in the padding (offset + size_unit * nb_unit > end_padding")

        new_member = StructMember(offset, nb_unit * size_unit)
        new_member.name = forms.name
        sub_struct = Struct(None, is_sub=True)
        new_member.set_array_struct(nb_unit, size_unit, [], sub_struct)
        sub_struct.add_pad()
        sub_struct.name = forms.type
        new_member.web_t = "array of %s" % (sub_struct.name)
        
        return new_member


    def add_member_from_web_ui(self, pad_member, forms):
        member_type = forms.member_type
        offset = forms.offset
        try:
            offset = int(offset)
        except ValueError:
            raise ValueError("Offset is not a integer")
        
        if offset < pad_member.offset or offset >= pad_member.offset + pad_member.size:
            raise ValueError("Offset has to be between padding_offset and padding_offset + padding_size")

        pad_idx = self.members.index(pad_member)
        
        if member_type == "simple":
            new_member = self.create_simple(forms, offset,
                                            pad_member.offset + pad_member.size)
        elif member_type == "array":
            new_member = self.create_array(forms, offset,
                                           pad_member.offset + pad_member.size)
        elif member_type == "struct":
            new_member = self.create_struct(forms, offset,
                                            pad_member.offset + pad_member.size)
        elif member_type == "array_struct":
            new_member = self.create_array_struct(forms, offset,
                                                  pad_member.offset + pad_member.size)
        else:
            raise ValueError("Bad member_type")

        self.members.insert(pad_idx, new_member)
        
    def get_best_size(self, accesses):
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

        return min([sz for sz in sizes if sizes[sz] == max_hit])

    def get_type(self, accesses, size):

        types = {}
        for access in accesses:
            t = access.analyse_ctx(size)
            if t and t in types.keys():
                types[t] += access.nb_access
            elif t:
                types[t] = access.nb_access

        _dynStruct.Access.remove_instrs(accesses)
        # ptr_struct_str and ptr_array_str are ptr_str with a comment
        # they provide more information but are the same C type than ptr_str
        # So they have to be prioritary on ptr_str
        if _dynStruct.ptr_str in types and\
           (_dynStruct.ptr_array_str or _dynStruct.ptr_struct_str):
            types[_dynStruct.ptr_str] = 0

        if not len(types):
            return "int%d_t" % (size * 8)

        return max(types.keys(), key=lambda x: types[x])

    def filter_access(self, accesses):
        accesses_cpy = list(accesses)
        for access in accesses_cpy:
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
        self.members = [StructMember(0, self.size)]
        self.members[0].set_array(self.size, 1)
        self.members[0].t = 'char'

    def struct_is_equal(self, struct):
        if struct.size != self.size:
            return False

        for other_member in struct.members:
            member = self.get_member(other_member.offset)
            if not member:
                continue
            if not (member.offset == other_member.offset and\
                    member.size == other_member.size):
                return False

            #If the only diff is type, check if there are ptr with comment and ptr
            types = [member.t, other_member.t]
            if not types[0] == types[1] and \
               not ((types[0].startswith(_dynStruct.ptr_str) or\
                     types[0] == _dynStruct.ptr_func_str) and\
                    (types[1].startswith(_dynStruct.ptr_str) or\
                     'int%d_t' % _dynStruct.bits in types[1])) and\
                not ((types[1].startswith(_dynStruct.ptr_str) or \
                      types[1] == _dynStruct.ptr_func_str) and\
                     (types[0].startswith(_dynStruct.ptr_str) or\
                      'int%d_t' % _dynStruct.bits in types[0])):
                return False

        return True

    def merge(self, struct):
        for other_member in struct.members:
            member = self.get_member(other_member.offset)
            if not member:
                self.insert_member(copy.deepcopy(other_member))
                continue

            # only case possible: change on ptr type
            # check which ptr we should keep and if other_member
            # replace it in self.members
            if member.t != other_member.t:
                if member.t == 'int%d_t' % _dynStruct.bits or\
                   member.t == _dynStruct.ptr_str:
                    self.members.remove(member)
                    self.insert_member(copy.deepcopy(other_member))

    def has_member_or_padding(self, offset, size, t):
        member = self.get_member(offset)

        # if member replace by padding
        if not member:
            return True
        if member.offset == offset and member.size == size and member.t == t:
            return True
        return False

    def add_block(self, block):
        self.blocks.append(block)
        block.struct = self

    def remove_block(self, block):
        self.blocks.remove(block)
        block.struct = None

    def remove_all_block(self):
        blocks_tmp = list(self.blocks)
        for block in blocks_tmp:
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
        non_padding = [m for m in self.members if not m.is_padding]

        if len(non_padding) < 2 and not non_padding[0].is_array_struct\
           and not non_padding[0].is_struct:
            return True

        if not True in [m.is_struct or m.is_array_struct or\
                         m.is_sub_struct for m in non_padding]:
            if False in [m.t == non_padding[0].t for m in non_padding]:
                return False

            # this a struct with only multiple simple member of the same type
            # and padding. This is not really a structure
            return True

        return False

    def detect(self, blocks):
        for block in blocks:
            if block.size != self.size or block.struct or \
               (not block.w_access and not block.r_access):
                continue
            tmp_struct = Struct(block)
            if self.struct_is_equal(tmp_struct):
                self.add_block(block)

    @staticmethod
    def recover_all_struct(blocks, structs):        
        prbar = pyprind.ProgBar(len(blocks), track_time=False, title="\nRecovering structures")
        for block in blocks:
            if not block.w_access and not block.r_access:
                continue

            tmp_struct = Struct(block)
            for struct in structs:
                if struct.struct_is_equal(tmp_struct):
                    struct.merge(tmp_struct)
                    struct.add_block(block)
                    break;

            if block.struct == tmp_struct:
                structs.append(tmp_struct)

            if len(structs) == 0:
                continue

            if len(structs[-1].members) == 0:
                structs[-1].remove_all_block()
                structs.remove(structs[-1])
            else:
                structs[-1].id = len(structs)
                structs[-1].set_default_name()
            prbar.update()
        print('\n')

    @staticmethod
    def clean_all_struct(structs, cleaning):
        list_structs = list(structs)
        for struct in list_structs:
            struct.clean_struct()
            if cleaning and struct.not_a_struct():
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
            member = struct.get_member(int(id_struct))
            if next_struct:
                return Struct.get_by_id(next_struct, member.sub_struct)
            if member.is_sub_struct:
                member = member.sub_struct
            return member

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
            if n == 0:
                tmp_name = '<a href="/struct?id=%s">' % (id_struct_split[0])
            else:
                tmp_name = '<a href="/member?id_struct=%s&id_member=%s">' % ('.'.join(id_struct_split[:n]), id_struct_split[n])
            tmp_name += Struct.get_by_id('.'.join(id_struct_split[:n+1])).name
            tmp_name += "</a>"
            name.append(tmp_name)
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
