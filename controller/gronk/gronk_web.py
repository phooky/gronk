import multipart
import os.path
import tempfile
import multiprocessing
from jinja2 import Template
from multiprocessing.connection import Client

address = ("localhost", 6543)

def update_file(file_data):
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


base_templ = Template(open('templates/base.html.jinja').read())

def application(environ, start_response):
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
            (_, ext) = os.path.splitext(name)
            ext = ext.lower()
            if ext == '.gcode':
                pass
            elif ext == '.svg':
                pass
            else:
                raise RuntimeError("Unable to process files of type '"+ext+"'. Please make sure your file has a .gcode or .svg extension.")
            update_file(curfile)
            return base_templ.render(curfile=curfile).encode('utf-8')
        except Exception as e:
            return error_templ.format(files,str(e)).encode('utf-8')
    else:
        start_response("200 OK", [("Content-Type", "text/html")])
        return base_templ.render(curfile=curfile).encode('utf-8')



