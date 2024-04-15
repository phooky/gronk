import RPi.GPIO as GPIO
import time
from queue import Queue
from threading import Thread, Event

class Buttons:
    rows = [ 12, 22, 18 ]
    columns = [5, 6, 13]
    keymap = [ "S+", "OK", "-", "P-", "S-", "P+", "TEST", "SET", "ONLINE" ]

    def __init__(self):
        GPIO.setmode(GPIO.BCM)
        for pin in self.rows+self.columns:
            GPIO.setup(pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)
        self.previous = 0
        self.evtq = Queue()
        self.thread = Thread(target=self.run)
        self.running = Event()
        self.running.clear()

    def scan_row(self,row_pin):
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(row_pin,GPIO.OUT,initial=GPIO.HIGH)
        v = 0
        time.sleep(0.01)
        for pin in self.columns:
            v = v << 1
            if GPIO.input(pin):
                v = v + 1
        time.sleep(0.01)
        GPIO.setup(row_pin,GPIO.IN)
        time.sleep(0.01)
        return v

    def scan(self):
        v = 0
        for row in self.rows:
            v = v << 3
            v = v + self.scan_row(row)
        for i in range(9):
            if (v&(1<<i)) != (self.previous&(1<<i)):
                if v&(1<<i):
                    #print(f"{keymap[i]} down")
                    self.evtq.put((self.keymap[i],1))
                else:
                    #print(f"{keymap[i]} up")
                    self.evtq.put((self.keymap[i],0))
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
