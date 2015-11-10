

def get_header(l_struct):
    s = ""
    for struct in l_struct:
        s += str(struct) + "\n"
    return s
        
def print_to_file(filename, l_struct):
    def_name = filename.upper().replace(".", "_") + "_"
    s = "#ifndef %s\n#define %s\n" % (def_name, def_name)
    s += get_header(l_struct) + "\n#endif"
    with open(filename, "w") as f:
        f.write(s)


def print_to_console(l_struct):
    print(get_header(l_struct))
