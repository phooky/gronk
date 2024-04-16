# Controller stuff

Everything in the "gronk" diretory gets deployed

Everything in "web" ends up in /var/www/gronk


# Testing web

 /usr/bin/uwsgi_python3 --http-socket localhost:9090 --wsgi-file ./gronk/gronk_web.py
 
 
