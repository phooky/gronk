#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
import gpiozero 
GPIO.setmode(GPIO.BCM)

rows = [ 12, 22, 18 ]
#columns = [ 23, 24, 25 ]
columns = [5, 6, 13]
#led = 26
test=27
GPIO.setup(test,GPIO.IN)
GPIO.add_event_detect(test, GPIO.RISING)
test_button = gpiozero.Button(24, pull_up = False)

def init():
    for pin in rows+columns:
        GPIO.setup(pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)




def scan_row(row_pin):
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

def scan():
    v = 0
    for row in rows:
        v = v << 3
        v = v + scan_row(row)
    return v

keymap = [ "S+", "OK", "-", "P-", "S-", "P+", "TEST", "SET", "ONLINE" ]

init()

previous = 0

def update(v):
    global previous
    for i in range(9):
        if (v&(1<<i)) != (previous&(1<<i)):
            if v&(1<<i):
                print(f"{keymap[i]} down")
            else:
                print(f"{keymap[i]} up")
    previous = v


try:
    while True:
        time.sleep(0.05)
        v = scan()
        update(v)
finally:
    GPIO.cleanup()



