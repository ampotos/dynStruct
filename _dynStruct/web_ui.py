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
        block = _dynStruct.l_block[block_id].__dict__.copy()
        block.pop("r_access")
        block.pop("w_access")
        return bottle.template("block_view", block=block)
    else:
        return bottle.template("error", msg="Bad block id")

@bottle.route("/access_search")
def access_view():
    id_block = bottle.request.query.id_block
    id_struct = bottle.request.query.id_struct

    id_block = int(id_block) if id_block else None
    id_struct = int(id_struct) if id_struct else None

    if id_block and (id_block < 0 or id_block >= len(_dynStruct.l_block)):
            return bottle.template("error", msg="Bad block id")
    elif id_struct and (id_struct < 0 or id_struct >= len(_dynStruct.l_struct)):
            return bottle.template("error", msg="Bad struct id")
    else:
        return bottle.template("access_search", id_block = id_block, id_struct=id_struct)

@bottle.route("/access_get")
def access_get():
    id_block = bottle.request.query.id_block
    id_struct = bottle.request.query.id_struct

    id_block = int(id_block) if id_block != "None" else None
    id_struct = int(id_struct) if id_struct != "None" else None

    return _dynStruct.access_json(id_block, id_struct)
    
@bottle.route("/static/<filename:path>")
def serve_static(filename):
    return bottle.static_file(filename, root=os.path.join(os.path.dirname(__file__), "static"))

def start_webui(addr, port):
    bottle.TEMPLATE_PATH.insert(0, os.path.dirname(__file__) + "/views")
    bottle.run(host=addr, port=port)
