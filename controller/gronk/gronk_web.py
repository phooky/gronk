import multipart
import os.path
import tempfile
import multiprocessing
from jinja2 import Template
from multiprocessing.connection import Client
import subprocess
import uwsgi


# This is the address of the local gronk interface
address = ("localhost", 6543)

def update_file(file_data):
    print("updating -- ",file_data)
    with Client(address, authkey = b"gronk") as connection:
        connection.send(file_data)
        #response = connection.recv()
        #print(response)    
        return True
    return False

error_templ = """
<html>
<head><title>GRONK ONLINE</title></head>
<body>
<h1>ERROR</h1>
{}
<p>Details: {}
</body>
</html>
"""

VPYPE_LOC = os.environ.get('VPYPE_LOC','/home/nycr/.local/bin/vpype')
GRONK_LOC = os.environ.get('GRONK_LOC','/home/nycr/gronk')


def spooler(env):
    print(" GOT SPOOLER ")
    print(env)
    svg_path = env[b'path']
    gcode_path = "/tmp/newtest.gcode" # svg_path + b".vpype.gcode"
    try:
        command = [VPYPE_LOC, '-c', GRONK_LOC+'/vpype-gcode.toml', 'read', '--quantization', '0.5mm',
                   svg_path.decode(), 'gwrite', gcode_path]
        print("RUNNING VPYPE", command)
        process = subprocess.run(command, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        print(process)
        print("FINISHED",gcode_path)
        update_file(("nonsense",gcode_path))
    except Exception as e:
        print("PROBLEM")
        print(e)
    finally:
        print("CONVERT COMPLETE ********************")
    return uwsgi.SPOOL_OK
    
uwsgi.spooler = spooler


base_templ = Template(open('templates/base.html.jinja').read())

def application(environ, start_response):
    curfile = None
    method = environ["REQUEST_METHOD"]
    print(" got ",method)
    if method == 'POST':
        return upload(environ, start_response)
    else:
        start_response("200 OK", [("Content-Type", "text/html")])
        return base_templ.render(curfile=curfile).encode('utf-8')


def upload(environ, start_response):
    print("UPLOAD")
    curfile = None
    method = environ["REQUEST_METHOD"]
    def on_field(field):
        pass
    def on_file(file):
        nonlocal curfile
        name = file.file_name
        (_, name) = os.path.split(file.file_name.decode())
        file.flush_to_disk()
        curfile = (name,file.actual_file_name)
    if method == 'POST':
        start_response("200 OK", [("Content-Type", "text/html")])
        multipart_headers = {'Content-Type': environ['CONTENT_TYPE']}
        multipart_headers['Content-Length'] = environ['CONTENT_LENGTH']
        try:
            multipart.parse_form(multipart_headers, environ['wsgi.input'], on_field, on_file)
            name = curfile[0]
            path = curfile[1]
            (_, ext) = os.path.splitext(name)
            ext = ext.lower()
            if ext == '.gcode':
                update_file(curfile)
            elif ext == '.svg':
                gcpath = path + '-convert.gcode'
                uwsgi.spool({b'path':path,b'outpath':gcpath})
            else:
                raise RuntimeError("Unable to process files of type '"+ext+"'. Please make sure your file has a .gcode or .svg extension.")
            return base_templ.render(curfile=curfile).encode('utf-8')
        except Exception as e:
            return error_templ.format(curfile,str(e)).encode('utf-8')



