import pickle
import _dynStruct

def get_header(l_struct):
    if len(l_struct) == 0:
        return "No structure found"
    s = ""
    for struct in l_struct:
        s += str(struct) + "\n"
    return s

def print_to_file(filename, l_struct):
    def_name = def_name.upper().replace(".", "_") + "_"
    s = "#ifndef %s\n#define %s\n" % (def_name, def_name)
    s += get_header(l_struct, def_name)
    s += "\n#endif"
    with open(filename, "w") as f:
        f.write(s)


def print_to_console(l_struct):
    print(get_header(l_struct))

def save_pickle(filename, l_struct, l_block, l_access_w, l_access_r):
    data = {"structs" : l_struct, "blocks" : l_block}
    data["w_access"] = l_access_w
    data["r_access"] = l_access_r
    with open(filename, "wb") as f:
        pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)

def save_modif():
    if _dynStruct.serialized_file:
        save_pickle(_dynStruct.serialized_file, _dynStruct.l_struct,
                    _dynStruct.l_block, _dynStruct.l_access_w,
                    _dynStruct.l_access_r)
