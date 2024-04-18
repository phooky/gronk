# Setting up the controller for Gronk

## Hardware

### Materials

* Raspberry Pi Zero W
* Waveshare 2.13" e-ink display V2
* USB On-the-go micro B to A adapter

## Controller

* Latest 32-bit Raspios (bookworm); do not use older versions
  (bullseye) despite what rpi-imager recommends
  "RASPBERRY PI OS LITE (32-BIT)"
* Install python3-pil
* Install nginx
* Install uwsgi uwsgi-plugin-python3 pipx
* Install picocom
* Install python3-serial
* Install python3-multipart python3-jinja2 python3-numpy
* Install git
* Set up

```
sudo apt-get install libopenblas-dev python3-pil nginx uwsgi uwsgi-plugin-python3 pipx picocom python3-serial python3-multipart python3-jinja2 python3-numpy python3-shapely git

pip install vpype vpype-gcode --break-system-packages
```
