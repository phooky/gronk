import multipart
import os.path
from os import makedirs, rename
import tempfile
import multiprocessing
from jinja2 import Template
from multiprocessing.connection import Client
import subprocess
import uwsgi
import traceback

# This is the address of the local gronk interface
address = ("localhost", 6543)

# Update the current queued file on Gronk. The file data is a tuple
# of (displayname, full gcode path).
def update_gronk_file(file_data):
    with Client(address, authkey = b"gronk") as connection:
        connection.send(file_data)
        return True
    return False

# Ask Gronk what the current queued file is. The file data is a
# tuple of (displayname, full gcode path) or None.
def query_gronk_file():
    with Client(address, authkey = b"gronk") as connection:
        connection.send("Q")
        response = connection.recv()
        print("Response to query",response)
        return True
    return False


VPYPE_LOC = os.environ.get('VPYPE_LOC','/home/nycr/.local/bin/vpype')
GRONK_LOC = os.environ.get('GRONK_LOC','/home/nycr/gronk')

# Gronk's cache of gcode resides in this directory.
GRONK_PLOTS_LOC = os.path.join(GRONK_LOC,'plots')
# Ensure that path exists!
makedirs(GRONK_PLOTS_LOC, exist_ok=True)

# Original full path; final path is constructed and returned
def install_gcode(name,path):
    (filename, _) = os.path.splitext(name)
    final_path = os.path.join(GRONK_PLOTS_LOC,filename + '.gcode')
    rename(path, final_path)
    return (name,final_path)

# This is the process that converts SVGs to GCODE.
def spooler(env):
    svg_path = env[b'path'].decode()
    gcode_path = env[b'outpath'].decode()
    name = env[b'name'].decode()
    try:
        command = [VPYPE_LOC, '-c', GRONK_LOC+'/vpype-gcode.toml', 'read', '--quantization', '0.5mm',
                   svg_path, 'gwrite', gcode_path]
        print("RUNNING VPYPE", command)
        process = subprocess.run(command, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        print(process)
        print("FINISHED",gcode_path)
        update_file(install_gcode(name,gcode_path))
    except Exception as e:
        print("PROBLEM")
        print(e)
    finally:
        print("CONVERT COMPLETE ********************")
    return uwsgi.SPOOL_OK
    
uwsgi.spooler = spooler


base_templ = Template(open('templates/base.html.jinja').read())
error_templ = Template(open('templates/error.html.jinja').read())

def build_page(curfile=None,spoolfile=None):
    return base_templ.render(curfile=curfile,spoolfile=spoolfile).encode('utf-8')
    

def application(environ, start_response):
    method = environ["REQUEST_METHOD"]
    print(" got ",method)
    start_response("200 OK", [("Content-Type", "text/html")])
    if method == 'POST':
        return upload(environ, start_response)
    else:
        return build_page(curfile=None)

def upload(environ, start_response):
    curfile = None
    def on_field(field):
        pass
    def on_file(file):
        nonlocal curfile
        name = file.file_name.decode()
        (_, name) = os.path.split(file.file_name.decode())
        file.flush_to_disk()
        curfile = (name,file.actual_file_name.decode())
    multipart_headers = {'Content-Type': environ['CONTENT_TYPE']}
    multipart_headers['Content-Length'] = environ['CONTENT_LENGTH']
    try:
        spoolfile = None
        multipart.parse_form(multipart_headers, environ['wsgi.input'], on_field, on_file)
        name = curfile[0]
        path = curfile[1]
        (basename, ext) = os.path.splitext(name)
        ext = ext.lower()
        if ext == '.gcode':
            update_file(curfile)
        elif ext == '.svg':
            gcpath = path + '-convert.gcode'
            spoolfile = uwsgi.spool({b'path':path.encode('utf-8'),
                                     b'outpath':gcpath.encode('utf-8'),
                                     b'name':basename.encode('utf-8')})
        else:
            raise RuntimeError("Unable to process files of type '"+ext+"'. Please make sure your file has a .gcode or .svg extension.")
        return build_page(curfile=curfile,spoolfile=spoolfile)
    except Exception as e:
        return error_templ.render(exception = traceback.format_exc()).encode('utf-8')



