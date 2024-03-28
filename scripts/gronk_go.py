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

Mode = Enum('Mode', ['IDLE','JOG','RUN'])

buttons = Buttons()
buttons.start()

gronk = Gronk('/dev/ttyACM0')

epd = EPD()
# resolution is 250x122

font_logo = ImageFont.truetype('./fonts/ArchivoBlack-Regular.ttf',26)
font_txt = ImageFont.truetype('./fonts/static/ArchivoNarrow-Regular.ttf',16)

# Todo -- status should return pen state!
pendown = None # pen starts in indeterminate state; assume down

def show_ready(path = None):
    epd.init(epd.FULL_UPDATE)
    epd.Clear(0xff)
    image = Image.new('1', (epd.height,epd.width),255)
    draw = ImageDraw.Draw(image)
    draw.text((10,5),'gronk.', font=font_logo, fill=0)
    draw.text((0,40),'Arrow keys to jog', font=font_txt, fill=0)
    draw.text((0,60),'TEST toggles pen up/down', font=font_txt, fill=0)
    draw.text((0,80),'Hold "ONLINE" for 5 seconds to shut down.', font=font_txt, fill=0)
    draw.text((0,100),'Upload files at http://gronk.lan/', font=font_txt, fill=0)
    epd.display(epd.getbuffer(image))
    epd.sleep()

def show_goodbye():
    epd.init(epd.FULL_UPDATE)
    epd.Clear(0xff)
    image = Image.new('1', (epd.height,epd.width),255)
    draw = ImageDraw.Draw(image)
    draw.text((10,5),'goodbye, gronk.', font=font_logo, fill=0)
    draw.text((5,40),'Please wait thirty seconds before', font=font_txt, fill=0)
    draw.text((5,60),'switching off.', font=font_txt, fill=0)
    epd.display(epd.getbuffer(image))
    epd.sleep()



show_ready()

shutdown_timer = None
running = Event()
running.set()

def shutdown():
    print("SHUTDOWN NOW")
    buttons.stop()
    running.clear()
    buttons.evtq.put(('SHUTDOWN',0))

try:
    jogs = [0,0]
    while running.is_set():
        (key,down) = buttons.evtq.get()
        print(key,down)
        if key == 'SHUTDOWN':
            gronk.enable_steppers(False)
            break
        if key == 'TEST' and down:
            if pendown == None:
                pendown = True
            if pendown:
                gronk.send("M04")
            else:
                gronk.send("M03")
            pendown = not pendown
        elif key == 'S+':
            jogs[1] = 1*down
        elif key == 'S-':
            jogs[1] = -1*down
        elif key == 'P-':
            jogs[0] = -1*down
        elif key == 'P+':
            jogs[0] = 1*down
        elif key == "ONLINE":
            if down:
                print("starting timer.")
                shutdown_timer = Timer(5,shutdown)
                shutdown_timer.start()
            else:
                print("timer cancelled.")
                shutdown_timer.cancel()
            
        en = jogs[0] != 0 or jogs[1] != 0
        print(f"key {key} en {en} jog0 {jogs[0]} jog1 {jogs[1]}")
        gronk.enable_steppers(en)
        gronk.jog(jogs[0],jogs[1])
        time.sleep(0.05)
finally:
    buttons.stop()

if not running.is_set():
    print("Full shutdown")
    show_goodbye()
    time.sleep(2);
    subprocess.Popen(['sudo','shutdown','-h','now'])

GPIO.cleanup()


