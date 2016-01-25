import os
import bottle

@bottle.route("/")
def index():
    return bottle.template("index")

@bottle.route("/static/<filename>")
def serve_static(filename):
    return bottle.static_file(filename, root=os.path.join(os.path.dirname(__file__), "static"))

def start_webui(l_struct, l_block, l_access_w, l_access_r, addr, port):
    bottle.TEMPLATE_PATH.insert(0, os.path.dirname(__file__) + "/views")
    print(bottle.TEMPLATE_PATH)
    bottle.run(host=addr, port=port)
