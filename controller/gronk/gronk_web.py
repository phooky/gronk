import multipart
import os.path

template = """
<html>
<head>
    <title>GRONK ONLINE, the giant plotter online service</title>
</head>
<body>
    <h1>This is GRONK, NYCR's largest-format plotter.</h1>
You can send SVG or raw GCode files to GRONK. GRONK uses its own special GCode
    <form method="post" enctype="multipart/form-data">
        <label for="file">File to plot:</label>
        <input id="file" name="file" type="file" />
        <button>Send SVG or GCODE file</button>
    </form>
    Request method was "{}".
</body>
</html>
"""

uploads_templ = """
<html>
<head><title>GRONK ONLINE</title></head>
<body>
<h1>GRONK has received your file.</h1>
<p>Details:
<p>Filename {}, Content type {}, content length {}.
</body>
</html>
"""


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


def application(environ, start_response):
    method = environ["REQUEST_METHOD"]
    files = {}
    def on_field(field):
        pass
    def on_file(file):
        name = file.file_name
        (_, name) = os.path.split(name)
        (_, ext) = os.path.splitext(name)
        fi = file.file_object
        f=open(os.path.join('/var/www/gronk/uploads',name.decode()),"wb")
        while True:
            d = fi.read(2048)
            if not d:
                break
            f.write(d)
            f.close()
        files[file.field_name] = {'name': file.file_name, 'object': file.file_object}    
    if method == 'POST':
        start_response("200 OK", [("Content-Type", "text/html")])
        multipart_headers = {'Content-Type': environ['CONTENT_TYPE']}
        multipart_headers['Content-Length'] = environ['CONTENT_LENGTH']
        try:
            multipart.parse_form(multipart_headers, environ['wsgi.input'], on_field, on_file)
            (_, name) = os.path.split(files[b'file']['name'])
            (_, ext) = os.path.splitext(name.decode())
            ext = ext.lower()
            if ext == '.gcode':
                f=open(os.path.join('/var/www/gronk/uploads',name.decode()),"wb")
                fi = files[b'file']['object']
                while True:
                    d = fi.read(2048)
                    stuff = stuff + " read {}".format(len(d))
                    if not d:
                        break
                    f.write(d)
                fi.close()
                f.close()
            elif ext == '.svg':
                pass
            else:
                raise RuntimeError("Unable to process files of type '"+ext+"'. Please make sure your file has a .gcode or .svg extension.")
        except Exception as e:
            return error_templ.format(files,str(e)).encode('utf-8')
        return uploads_templ.format(
            stuff,
            environ["CONTENT_TYPE"],
            environ["CONTENT_LENGTH"]).encode('utf-8')
    else:
        start_response("200 OK", [("Content-Type", "text/html")])
        return template.format(environ["REQUEST_METHOD"]).encode('utf-8')

