import binascii
import _dynStruct

class Access:

    def __init__(self, access, orig, addr_start, block, id_access):
        self.block = block
        self.offset = access
        self.addr = addr_start + self.offset
        self.size = orig["size_access"]
        self.id_access = id_access

        if len(orig["opcode"]) % 2:
            orig["opcode"] = "0" + orig["opcode"]
        self.instr_op = orig["opcode"]

        if orig["ctx_opcode"] and len(orig["ctx_opcode"]) % 2:
            orig["ctx_opcode"] = "0" + orig["ctx_opcode"]
        self.ctx_opcode = orig["ctx_opcode"]

        json_attrib = ["nb_access", "pc", "func_pc",
                       "func_sym", "func_module", "ctx_addr"]
        
        for k in json_attrib:
            setattr(self, k, (orig[k]))

        self.instr = [instr for instr in
                      _dynStruct.disasm.disasm(binascii.unhexlify(self.instr_op),
                                               self.pc)][0]

        if self.ctx_opcode:
            self.ctx_instr = [instr for instr in
                              _dynStruct.disasm.disasm(binascii.unhexlify(self.ctx_opcode),
                                                       self.ctx_addr)][0]
        else:
            self.ctx_instr = None

    def is_offset(self, offset):
        return self.offset == offset

    def is_in_range(self, start, end):
        if self.offset >= start and self.offset < end:
            return True

        if self.offset < start and self.offset + self.size >start:
            return True

        return False
        
    def in_member(self, member):
        if self.is_offset(member.offset):
            return True

        if self.offset >= member.offset and\
           self.offset < member.offset + member.size:
            return True

        return False
