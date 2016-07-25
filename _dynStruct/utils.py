import _dynStruct
import capstone

# active detail to operand information (used to analize context instar)
def create_disasm():
        if _dynStruct.disasm:
            return

        if _dynStruct.bits == 64:
            _dynStruct.disasm = capstone.Cs(capstone.CS_ARCH_X86,
                                            capstone.CS_MODE_64)
        else:
            _dynStruct.disasm = capstone.Cs(capstone.CS_ARCH_X86,
                                            capstone.CS_MODE_32)
        _dynStruct.disasm.detail = True
