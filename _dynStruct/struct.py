from .struct_member import StructMember

# todo add detection of array of Struct

# list of classical string manipulation function
# use to detecte string
str_func = ["strlen", "strcpy", "strncpy", "strcmp", "strncmp", "strdup"]

# list of classical function which we ignore the access
# because there access are non relative to the struct
ignore_func = ["memset", "memcpy", "memcmp"]

# size where we consider the struct is an array
# if all members have the same size
size_array = 5

class Struct:


    def __init__(self, block):
        self.name = ""
        self.size = block.size
        self.blocks = []
        self.looks_array = False;
        self.size_array_unit = 0
        self.members = []

        self.recover(block)
        self.add_block(block)
        
    def __str__(self):
        # for string we don't anything
        # for other tab it's hard to say if it's an array are a struct
        if self.looks_array and (self.size_array_unit == 1 or\
                                 len(self.members) >= size_array):
            s = "uint%d_t %s[%d];\n" % (self.size_array_unit * 8,\
                                      self.name, len(self.members))
            return s
            
        s = "typedef struct {\n"
        for member in self.members:
            s += "\t" + str(member)
        s += "} %s;\n" % self.name
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
            
            size_member = self.get_better_size(block, actual_offset, accesses)
            self.members.append(StructMember(actual_offset, size_member, block))
            actual_offset += size_member
            
        if self.look_like_array():
            self.looks_array = True
            self.size_array_unit = self.members[0].size
            
    # maybe check only the write accesses if the actual result is too bad
    def get_better_size(self, block, offset, accesses):
        sizes = {}

        for access in accesses:
            if access.size in sizes:
                sizes[access.size] += 1
            else:
                sizes[access.size] = 1

        max_hit = 0
        for size in range(len(sizes)):
            if size in sizes and sizes[size] > max_hit:
                max_hit = sizes[size]

        return max(sizes)
        
    def filter_access(self, accesses):
        for access in accesses:
            for func in ignore_func:
                if func.lower() in access.func_sym.lower():
                    accesses.remove(access)

    def has_str_access(self, accesses):
        for access in accesses:
            for func in ignore_func:
                if func.lower() in access.func_sym.lower():
                    return True
        return False
                    
    def change_to_str(self, block):
        self.members.clear()
        for offset in range(self.size):
            self.members.append(StructMember(offset, 1, block))
    
    def block_is_struct(self, block):
        if block.size != self.size:
            return False

        actual_offset = 0
        while actual_offset != self.size:
            accesses = block.get_access_by_offset(actual_offset)
            if not accesses:
                actual_offset += 1
                continue

            self.filter_access(accesses)
            if self.has_str_access(accesses):
                if self.looks_array and self.size_array_unit == 1:
                    return True
                
            size_member = self.get_better_size(block, actual_offset, accesses)
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
        return

    def look_like_array(self):

        if not len(self.members):
            return False

        size = self.members[0].size

        for member in self.members:
            if member.size != size:
                return False

        return True

    def maj_member():
        for member in members:
            member.access.clear()
            member.get_accesses()

            if self.look_like_array():
                self.looks_array = True
                self.size_array_unit = self.members[0].size
            else:
                self.looks_array = False

    @staticmethod
    def recover_all_struct(blocks, structs):
        for block in blocks:
            for struct in structs:
                if struct.block_is_struct(block):
                    struct.add_block(block)
                    break;

            if not block.struct and (block.w_access or block.r_access):
                structs.append(Struct(block))

            if len(structs) == 0:
                continue

            if len(structs[-1].members) <= 1:
                structs.remove(structs[-1])
            else:
                if structs[-1].looks_array and len(structs[-1].members) >= size_array:
                    structs[-1].name = "array%d" % len(structs)
                else:
                    structs[-1].name = "struct%d" % len(structs)
