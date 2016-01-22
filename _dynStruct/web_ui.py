import os
from bottle import route
from bottle import run
from bottle import static_file

@route("/")
def index():
    return serve_static("index.html")

@route("/static/<filename>")
def serve_static(filename):
    return static_file(filename, root=os.path.join(os.path.dirname(__file__), 'static'))

def start_webui(l_struct, l_block, l_access_w, l_access_r, addr, port):
    run(host=addr, port=port)
