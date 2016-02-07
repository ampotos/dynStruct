import os
import bottle
import _dynStruct

@bottle.route("/")
def index():
    return bottle.template("index")

@bottle.route("/block")
def block_view():
    block_id = int(bottle.request.query.id)
    if block_id >= 0 and block_id < len(_dynStruct.l_block):
        return bottle.template("block_view", block=_dynStruct.l_block[block_id])
    else:
        return bottle.template("error", msg="Bad block id")

@bottle.route("/block_search")
def block_search():
    id_struct = bottle.request.query.id_struct

    id_struct = int(id_struct) if id_struct else None

    if id_struct != None and not _dynStruct.Struct.get_by_id(id_struct):
            return bottle.template("error", msg="Bad struct id")
    else:
        return bottle.template("block_search", id_struct=id_struct)

@bottle.route("/block_get")
def block_get():
    id_struct = bottle.request.query.id_struct

    id_struct = int(id_struct) if id_struct != "None" else None

    return _dynStruct.block_json(id_struct, bottle.request.query)

@bottle.route("/access_search")
def access_search():
    id_block = bottle.request.query.id_block
    id_member = bottle.request.query.id_member

    id_block = int(id_block) if id_block else None
    id_member = id_member if id_member else None

    if id_block != None and (id_block < 0 or id_block >= len(_dynStruct.l_block)):
            return bottle.template("error", msg="Bad block id")
    elif id_member != None and not _dynStruct.Struct.get_by_id(id_member):
            return bottle.template("error", msg="Bad struct id")
    else:
        return bottle.template("access_search", id_block=id_block, id_member=id_member)

@bottle.route("/access_get")
def access_get():
    id_block = bottle.request.query.id_block
    id_member = bottle.request.query.id_member

    id_block = int(id_block) if id_block != "None" else None
    id_member = id_member if id_member != "None" else None

    return _dynStruct.access_json(id_block, id_member, bottle.request.query)

@bottle.route("/struct")
def struct_view():
    struct = _dynStruct.Struct.get_by_id(bottle.request.query.id)
    
    if struct == None:
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

@bottle.route("/member")
def member_view():
    id_struct = bottle.request.query.id_struct
    id_member = bottle.request.query.id_member

    if not id_member:
        return bottle.template("error", msg="member id missing")

    member = _dynStruct.Struct.get_member_by_id(id_struct, int(id_member))
    if not member:
        return bottle.template("error", msg="bad member id")
    
    if id_struct == None or not  _dynStruct.Struct.get_by_id(id_struct):
        return bottle.template("error", msg="Bad struct id")

    return bottle.template("member_view",
                           id_member="%s.%d" % (id_struct, member.offset),
                           member=member,
                           name_member=_dynStruct.Struct.make_member_name(id_struct, member.offset))

@bottle.route("/static/<filename:path>")
def serve_static(filename):
    return bottle.static_file(filename, root=os.path.join(os.path.dirname(__file__), "static"))

def start_webui(addr, port):
    bottle.TEMPLATE_PATH.insert(0, os.path.dirname(__file__) + "/views")
    bottle.run(host=addr, port=port)
