#!/usr/bin/env python

from PIL import Image, ImageDraw, ImageFont
from epd2in13_V2 import EPD
from threading import Lock, Timer

# Display is waveshare 2.13" eHAT; documented here:
# https://www.waveshare.com/wiki/2.13inch_e-Paper_HAT_Manual
# Resolution is 250x122

class Display:
    def __init__(self):
        self.epd = EPD()
        self.sleeping = True
        self.lock = Lock()
        self.sleep_timeout = 300; # 5m without activity to sleep
        self.sleep_timer = None
        self.dim = (self.epd.height,self.epd.width) # EPD module has w/h confused :P
        self.image = Image.new('1', self.dim ,255)
        self.draw = ImageDraw.Draw(self.image)
        self.font_logo = ImageFont.truetype('./fonts/ArchivoBlack-Regular.ttf',26)
        self.font_txt = ImageFont.truetype('./fonts/static/ArchivoNarrow-Bold.ttf',16)
        
    def do_sleep(self):
        "Put display to sleep after timer runs out."
        with self.lock:
            self.sleep_timer = None
            self.sleeping = True
            self.epd.sleep()
            
    def update(self, clear=False):
        "Update from buffer and start sleep timer."
        with self.lock:
            # we have a potential race condition here, but at worst it just results
            # in an unecessary sleep() call at the end so it's not worth handling.
            if self.sleep_timer:
                self.sleep_timer.cancel()
                self.sleep_timer = None
            if self.sleeping:
                self.epd.init(self.epd.FULL_UPDATE)
                clear = True
                self.sleeping = False
            if clear:
                self.epd.Clear(0xff)
            self.epd.display(self.epd.getbuffer(self.image))
            self.sleep_timer = Timer(self.sleep_timeout,self.do_sleep)

    def clear(self):
        self.draw.rectangle([(0,0),self.dim],fill=255) # clear image

    def text(self,loc,text,font=None):
        if not font:
            font = self.font_txt
        self.draw.text(loc,text,font=font,fill=0)
    
    def show_ready(self,path = None):
        self.clear()
        self.text((0,0),'gronk is ready.', font=self.font_logo)
        if path:
            self.text((0,25),f'SET prints {path}')
        else:
            self.text((0,25),'Arrow keys to jog')
        self.text((0,45),'TEST toggles pen up/down')
        self.text((0,65),'Upload files at http://gronk.lan/')
        self.text((15,85),'Hold "ONLINE" button')
        self.text((15,105),'for 5 seconds to shut down.')
        self.update()
    


    def show_goodbye(self):
        self.clear()
        self.text((10,5),'goodbye, gronk.', font=self.font_logo)
        self.text((0,40),'Please wait thirty seconds before')
        self.text((0,60),'switching off.')
        self.text((0,80),'Expect to wait about 120 seconds for')
        self.text((0,100),'gronk to boot after turning back on.')
        self.update()


