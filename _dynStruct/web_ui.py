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
def block_view():
    # Todo:  search for id_block or id_struct in param
    # check there are correct and pass them to the template
    if bottle.request.query.iframe:
        return bottle.template("access_search_iframe")
    return bottle.template("access_search_alone")

@bottle.route("/static/<filename:path>")
def serve_static(filename):
    return bottle.static_file(filename, root=os.path.join(os.path.dirname(__file__), "static"))

def start_webui(addr, port):
    bottle.TEMPLATE_PATH.insert(0, os.path.dirname(__file__) + "/views")
    bottle.run(host=addr, port=port)
