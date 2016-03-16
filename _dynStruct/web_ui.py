import os
import bottle
import _dynStruct

def check_block_id(block_id):
    if not block_id or block_id == "None":
        return None
    try:
        block_id = int(block_id)
    except:
        return False
    if block_id < 0 or block_id >= len(_dynStruct.l_block):
        return False
    return block_id

def check_struct_id(id_struct):
    if not id_struct or id_struct == "None":
        return None
    if not _dynStruct.Struct.get_by_id(id_struct):
        return False
    return id_struct

def check_id_member_from_access(id_member):
    if not id_member or id_member == "None":
        return None
    if not _dynStruct.Struct.get_by_id(id_member):
        return False
    return id_member


@bottle.route("/")
def index():
    return bottle.template("index")

@bottle.route("/block")
def block_view():
    block_id = check_block_id(bottle.request.query.id)

    if block_id or block_id == 0:
        return bottle.template("block_view", block=_dynStruct.l_block[block_id])
    else:
        return bottle.template("error", msg="Bad block id")

@bottle.route("/block_search")
def block_search():
    id_struct = check_struct_id(bottle.request.query.id_struct)

    if id_struct == False:
            return bottle.template("error", msg="Bad struct id")
    else:
        return bottle.template("block_search", id_struct=id_struct)

@bottle.route("/block_get")
def block_get():
    id_struct = check_struct_id(bottle.request.query.id_struct)

    return _dynStruct.block_json(id_struct, bottle.request.query)
    
@bottle.route("/access_search")
def access_search():
    id_block = check_block_id(bottle.request.query.id_block)
    id_member = check_id_member_from_access(bottle.request.query.id_member)

    if id_block != 0 and id_block == False:
            return bottle.template("error", msg="Bad block id")
    elif id_member != 0 and id_member == False:
            return bottle.template("error", msg="Bad struct id")
    else:
        return bottle.template("access_search", id_block=id_block, id_member=id_member)

@bottle.route("/access_get")
def access_get():
    id_block = check_block_id(bottle.request.query.id_block)
    id_member = check_id_member_from_access(bottle.request.query.id_member)

    return _dynStruct.access_json(id_block, id_member, bottle.request.query)

@bottle.route("/struct")
def struct_view():
    struct = _dynStruct.Struct.get_by_id(bottle.request.query.id)

    if not struct:
        return bottle.template("error", msg="Bad struct id")    

    return bottle.template("struct_view", struct=struct)

@bottle.route("/struct_search")
def struct_search():
    return bottle.template("struct_search")

@bottle.route("/struct_get")
def struct_get():
    return _dynStruct.struct_json()

@bottle.route("/member_get")
def member_get():
    id_struct = bottle.request.query.id_struct
    return _dynStruct.member_json(_dynStruct.Struct.get_by_id(id_struct), id_struct)

def member_template(query, edit):
    id_struct = check_struct_id(query.id_struct)
    id_member = query.id_member

    if id_member != 0 and not id_member:
        return bottle.template("error", msg="member id missing")

    if id_struct != 0 and not id_struct:
        return bottle.template("error", msg="Bad struct id")
    id_struct = str(id_struct)

    member = _dynStruct.Struct.get_member_by_id(id_struct, int(id_member))

    if not member:
        return bottle.template("error", msg="bad member id")    

    return bottle.template("member_view",
                           id_member="%s.%d" % (id_struct, member.offset),
                           member=member,
                           name_member=_dynStruct.Struct.make_member_name(id_struct, member.offset),
                           edit=edit)
    
@bottle.route("/member")
def member_view():
    return(member_template(bottle.request.query, False))

@bottle.route("/member_edit")
def member_edit():
    return(member_template(bottle.request.query, True))

def edit_simple_member(forms, member, next_member):
    try:
        size = int(forms.size)  
    except ValueError:
        return bottle.template("error", msg="Size is not an integer")

    if size <= 0:
        return bottle.template("error", msg="Size cannot be negative or Null")
        
    if size > member.size and \
       (not next_member.is_padding or next_member.size + member.size < size):
        return bottle.template("error", msg="Size is too big (not enough padding between this member and the next one)")

    member.name = forms.name
    member.size = size
    member.t = forms.type
    member.web_t = member.t
    
    return None

def edit_struct_size(struct, new_size):
    if new_size != struct.size:
        if new_size < struct.size:
            old_struct_members = list(struct.members)
            for m in old_struct_members:
                if m.offset + m.size > new_size:
                    struct.members.remove(m)
        struct.size = new_size
        # to have correct padding after the change of size
        struct.add_pad()


def edit_array_struct(forms, member, next_member):
    try:
        size_unit = int(forms.size_unit)
        nb_unit = int(forms.nb_unit)
    except ValueError:
        return bottle.template("error", msg="size_unit or nb_unit is not an integer")

    if size_unit <= 0 or nb_unit <= 0:
        return bottle.template("error", msg="size_unit and nb_unit cannot be negative or Null")
        
    if size_unit * nb_unit > member.size and \
       (not next_member.is_padding or next_member.size + member.size < size_unit * nb_unit):
        return bottle.template("error", msg="Size is too big (not enough padding between this member and the next one)")

    member.name = forms.name
    member.number_unit = nb_unit
    member.size_unit = size_unit
    member.size = size_unit * nb_unit
    member.sub_struct.name = forms.type

    member.web_t = "array of %s" % (member.sub_struct.name)
    
    edit_struct_size(member.sub_struct, size_unit)
    
    return None

def edit_struct(forms, member, next_member):
    try:
        size = int(forms.size)  
    except ValueError:
        return bottle.template("error", msg="Size is not an integer")

    if size <= 0:
        return bottle.template("error", msg="Size cannot be negative or Null")
        
    if size > member.size and \
       (not next_member.is_padding or next_member.size + member.size < size):
        return bottle.template("error", msg="Size is too big (not enough padding between this member and the next one)")

    edit_struct_size(member.sub_struct, new_size)
    
    member.name = forms.name
    member.sub_struct.name = member.name
    member.size = size
    member.t = member.name
    member.web_t = member.t
    
    return None

@bottle.route("/member_do_edit", method='POST')
def member_do_edit():
    id_struct = check_struct_id(bottle.request.query.id_struct)
    id_member = bottle.request.query.id_member

    if id_member != 0 and not id_member:
        return bottle.template("error", msg="member id missing")

    if id_struct != 0 and not id_struct:
        return bottle.template("error", msg="Bad struct id")
    id_struct = str(id_struct)

    struct = _dynStruct.Struct.get_by_id(id_struct)    
    if not struct:
        return bottle.template("error", msg="bad struct id")

    member = struct.get_member(int(id_member))
    print(id_member)
    next_member = struct.get_member(int(id_member) + member.size)

    if member.is_array:
        # do array stuff
        return bottle.template("error", msg="Not yet implem")
    elif member.is_struct:
        # do array struct
        return bottle.template("error", msg="Not yet implem")
    elif member.is_array_struct:
        ret = edit_array_struct(bottle.request.forms, member, next_member)
    else:
        ret = edit_simple_member(bottle.request.forms, member, next_member)

    if ret:
        return ret

    # if next member is padding remove it, add_pad will set a new padding with
    # correct size + offset if needed
    if next_member.is_padding:
        struct.members.remove(next_member)

    struct.add_pad()
    _dynStruct.save_modif()
    bottle.redirect("/member?id_struct=%s&id_member=%s" % (id_struct, id_member))
    
@bottle.route("/header.h")
def dl_header():
    bottle.response.content_type = 'text/x-c'
    return _dynStruct.get_header(_dynStruct.l_struct)

@bottle.route("/static/<filename:path>")
def serve_static(filename):
    return bottle.static_file(filename, root=os.path.join(os.path.dirname(__file__), "static"))

@bottle.route("/remove_struct")
def remove_struct_from_block():
    id_block = check_block_id(bottle.request.query.id_block)

    if id_block != 0 and not id_block:
        return bottle.template("error", msg="Bad block id")

    _dynStruct.l_block[id_block].struct.remove_block(_dynStruct.l_block[id_block])
    _dynStruct.save_modif()
    bottle.redirect("/block?id=%d" % (id_block))

@bottle.route("/add_to_struct")
def add_to_struct_struct_from_block():
    id_block = check_block_id(bottle.request.query.id_block)

    if id_block != 0 and not id_block:
        return bottle.template("error", msg="Bad block id")
    return bottle.template("struct_select", id_block=id_block)

@bottle.route("/struct_select_get")
def get_list_compat_struct():
    id_block = check_block_id(bottle.request.query.id_block)
    return _dynStruct.struct_select_json(id_block)    

@bottle.route("/do_add_to_struct")
def link_block():
    id_block = check_block_id(bottle.request.query.id_block)
    id_struct = check_struct_id(bottle.request.query.id_struct)
    if id_block != 0 and not id_block:
        return bottle.template("error", msg="Bad block id")
    if id_struct != 0 and not id_struct:
        return bottle.template("error", msg="Bad struct id")

    _dynStruct.Struct.get_by_id(id_struct).add_block(_dynStruct.l_block[id_block])
    _dynStruct.save_modif()
    
    bottle.redirect("/block?id=%d" % (id_block))

def start_webui(addr, port):
    bottle.TEMPLATE_PATH.insert(0, os.path.dirname(__file__) + "/views")
    bottle.run(host=addr, port=port)
