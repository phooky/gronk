#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
import gpiozero
from enum import Enum
from gronklib import Gronk
from gronkbuttons import Buttons
from queue import Queue
from threading import Thread, Event, Timer
from PIL import Image, ImageDraw, ImageFont
from epd2in13_V2 import EPD
import subprocess
from multiprocessing.connection import Listener, Client

#
# Gronk_go listens to port 6543 for uploaded file information.
# The latest file is always presented as a tuple ("filename","full path");
#

Mode = Enum('Mode', ['IDLE','RUNNING','PAUSING','PAUSED'])

buttons = Buttons()
buttons.start()

mode = Mode.IDLE

gronk = Gronk('/dev/ttyACM0')

epd = EPD()
# resolution is 250x122

font_logo = ImageFont.truetype('./fonts/ArchivoBlack-Regular.ttf',26)
font_txt = ImageFont.truetype('./fonts/static/ArchivoNarrow-Bold.ttf',16)

# Todo -- status should return pen state!
pendown = None # pen starts in indeterminate state; assume down

def show_ready(path = None):
    epd.init(epd.FULL_UPDATE)
    epd.Clear(0xff)
    image = Image.new('1', (epd.height,epd.width),255)
    draw = ImageDraw.Draw(image)
    draw.text((0,0),'gronk is ready.', font=font_logo, fill=0)
    if path:
        draw.text((0,25),f'SET prints {path}', font=font_txt, fill=0)
    else:
        draw.text((0,25),'Arrow keys to jog', font=font_txt, fill=0)
    draw.text((0,45),'TEST toggles pen up/down', font=font_txt, fill=0)
    draw.text((0,65),'Upload files at http://gronk.lan/', font=font_txt, fill=0)
    draw.text((15,85),'Hold "ONLINE" button', font=font_txt, fill=0)
    draw.text((15,105),'for 5 seconds to shut down.', font=font_txt, fill=0)
    epd.display(epd.getbuffer(image))
    epd.sleep()

def show_goodbye():
    epd.init(epd.FULL_UPDATE)
    epd.Clear(0xff)
    image = Image.new('1', (epd.height,epd.width),255)
    draw = ImageDraw.Draw(image)
    draw.text((10,5),'goodbye, gronk.', font=font_logo, fill=0)
    draw.text((0,40),'Please wait thirty seconds before', font=font_txt, fill=0)
    draw.text((0,60),'switching off.', font=font_txt, fill=0)
    draw.text((0,80),'Expect to wait about 120 seconds for', font=font_txt, fill=0)
    draw.text((0,100),'gronk to boot after turning back on.', font=font_txt, fill=0)

    epd.display(epd.getbuffer(image))
    epd.sleep()



show_ready()

shutdown_timer = None
running = Event()
pausing = Event()
running.set()
full_shutdown = False

cur_file = None

def file_thread():
    global cur_file
    address = ("localhost", 6543)
    listener = Listener(address, authkey = b"gronk")
    while running.is_set():
        c = listener.accept()
        print("got connection")
        try:
            cur_file = c.recv()
            show_ready(cur_file[0])
        except Exception as e:
            print("Exception ",e)

ft = Thread(target=file_thread)
ft.start()

def shutdown():
    global full_shutdown
    print("SHUTDOWN NOW")
    buttons.stop()
    full_shutdown = True
    running.clear()
    # handle shutting down listener thread
    Client(("localhost", 6543), authkey = b"gronk")
    buttons.evtq.put(('SHUTDOWN',0))

def print_thread(path):
    global mode
    gronk.enable_steppers()
    f = open(path)
    for line in f.readlines():
        if not running.is_set():
            print("not running")
            break
        if pausing.is_set():
            mode = Mode.PAUSING
            print("pausing")
            while pausing.is_set() and running.is_set():
                time.sleep(0.2)
            mode = Mode.RUNNING
        else:
            gronk.send(line)
    gronk.s.flush()
    mode = Mode.IDLE

def start_print(path):
    global mode
    mode = Mode.RUNNING
    t = Thread(target = print_thread, args=(path,))
    t.start()

jogs = [0,0]
    
def handle_idle(keypress):
    global jogs
    global pendown
    (key, down) = keypress
    if key == 'TEST' and down:
        if pendown == None:
            pendown = True
        if pendown:
            gronk.send("M04")
        else:
            gronk.send("M03")
        pendown = not pendown
    elif key == 'OK':
        gronk.set_zero()
    elif key == 'S+':
        jogs[1] = 1*down
    elif key == 'S-':
        jogs[1] = -1*down
    elif key == 'P-':
        jogs[0] = -1*down
    elif key == 'P+':
        jogs[0] = 1*down
    elif key == 'SET' and down and cur_file:
        start_print(cur_file[1])
        return
    elif key == "ONLINE":
        if down:
            print("starting timer.")
            shutdown_timer = Timer(5,shutdown)
            shutdown_timer.start()
        else:
            print("timer cancelled.")
            shutdown_timer.cancel()
    en = jogs[0] != 0 or jogs[1] != 0
    if en:
        gronk.enable_steppers(en)
    gronk.jog(jogs[0],jogs[1])

    
def handle_running(keypress):
    global pendown
    (key, down) = keypress
    if key == 'TEST' and down:
        if pendown == None:
            pendown = True
        if pendown:
            gronk.send("M04")
        else:
            gronk.send("M03")
        pendown = not pendown
    elif key == 'ONLINE' and down:
        if pausing.is_set():
            pausing.clear()
        else:
            pausing.set()

try:
    while running.is_set():
        keypress = buttons.evtq.get()
        if mode == Mode.IDLE:
            handle_idle(keypress)
        elif mode == Mode.RUNNING or mode == mode.PAUSING or mode == Mode.PAUSED:
            handle_running(keypress)
        if keypress[0] == 'SHUTDOWN':
            gronk.enable_steppers(False)
            break
        time.sleep(0.05)
finally:
    buttons.stop()
    running.clear()
    # handle shutting down listener thread
    print("connecting to shutdown")
    Client(("localhost", 6543), authkey = b"gronk").send("")

if not full_shutdown:
    print("Full shutdown")
    show_goodbye()
    time.sleep(2);
    subprocess.Popen(['sudo','shutdown','-h','now'])

GPIO.cleanup()


