#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
import gpiozero
from enum import Enum
from gronklib import Gronk
from queue import Queue
from threading import Thread, Event
from PIL import Image, ImageDraw, ImageFont
from epd2in13_V2 import EPD

GPIO.setmode(GPIO.BCM)
rows = [ 12, 22, 18 ]
columns = [5, 6, 13]
keymap = [ "S+", "OK", "-", "P-", "S-", "P+", "TEST", "SET", "ONLINE" ]

Mode = Enum('Mode', ['IDLE','JOG','RUN'])

class GronkButtons:
    def __init__(self):
        for pin in rows+columns:
            GPIO.setup(pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)
        self.previous = 0
        self.evtq = Queue()
        self.thread = Thread(target=self.run)
        self.running = Event()
        self.running.clear()

    def scan_row(self,row_pin):
        GPIO.setup(row_pin,GPIO.OUT,initial=GPIO.HIGH)
        v = 0
        time.sleep(0.01)
        for pin in columns:
            v = v << 1
            if GPIO.input(pin):
                v = v + 1
        time.sleep(0.01)
        GPIO.setup(row_pin,GPIO.IN)
        time.sleep(0.01)
        return v

    def scan(self):
        v = 0
        for row in rows:
            v = v << 3
            v = v + self.scan_row(row)
        for i in range(9):
            if (v&(1<<i)) != (self.previous&(1<<i)):
                if v&(1<<i):
                    print(f"{keymap[i]} down")
                    self.evtq.put((keymap[i],1))
                else:
                    print(f"{keymap[i]} up")
                    self.evtq.put((keymap[i],0))
        self.previous = v

    def run(self):
        while self.running.is_set():
            time.sleep(0.05)
            self.scan()
            
    def start(self):
        GPIO.setmode(GPIO.BCM)
        self.running.set()
        self.thread.start()
        
    def stop(self):
        self.running.clear()

buttons = GronkButtons()
buttons.start()

gronk = Gronk('/dev/ttyACM0')

epd = EPD()
epd.init(epd.FULL_UPDATE)
epd.Clear(0xff)

font24 = ImageFont.truetype('./Font.ttc',24)
font14 = ImageFont.truetype('./Font.ttc',14)
image = Image.new('1', (epd.height,epd.width),255)
draw = ImageDraw.Draw(image)
draw.text((0,10),'I AM GRONK.', font=font24, fill=0)
draw.text((0,60),'Use the arrow keys to jog the pen.', font=font14, fill=0)
draw.text((0,90),'Send files to http://gronk.lan/', font=font14, fill=0)
epd.display(epd.getbuffer(image))
epd.sleep()
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



