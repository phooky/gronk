#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
import gpiozero
from enum import Enum
from gronklib import Gronk
from gronkbuttons import Buttons
from queue import Queue
from threading import Thread, Event
from PIL import Image, ImageDraw, ImageFont
from epd2in13_V2 import EPD

Mode = Enum('Mode', ['IDLE','JOG','RUN'])

buttons = GronkButtons()
buttons.start()

gronk = Gronk('/dev/ttyACM0')

epd = EPD()
epd.init(epd.FULL_UPDATE)
epd.Clear(0xff)
# resolution is 250x122
image = Image.new('1', (epd.height,epd.width),255)
draw = ImageDraw.Draw(image)

font_logo = ImageFont.truetype('./fonts/ArchivoBlack-Regular.ttc',24)
font14 = ImageFont.truetype('./font/ArchivoNarrow-Regular.ttc',12)

def show_ready(path = None):
    draw.text((10,5),'gronk.', font=font24, fill=0)
    draw.text((5,40),'Use the arrow keys to jog.', font=font14, fill=0)
    draw.text((5,65),'Press "ONLINE" to shut down.', font=font14, fill=0)
    draw.text((10,90),'Upload files at http://gronk.lan/', font=font14, fill=0)
    epd.display(epd.getbuffer(image))
    epd.sleep()


show_ready()

try:
    jogs = [0,0]
    while True:
        (key,down) = buttons.evtq.get()
        print("got key")
        if key == 'S+':
            jogs[1] = 1*down
        elif key == 'S-':
            jogs[1] = -1*down
        elif key == 'P+':
            jogs[0] = -1*down
        elif key == 'P-':
            jogs[0] = 1*down
        en = jogs[0] != 0 or jogs[1] != 0
        print(f"key {key} en {en} jog0 {jogs[0]} jog1 {jogs[1]}")
        gronk.enable_steppers(en)
        gronk.jog(jogs[0],jogs[1])
        time.sleep(0.05)
finally:
    buttons.stop()
    GPIO.cleanup()



