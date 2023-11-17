#!/usr/bin/env python
import math
import serial
import time
port = serial.Serial('/dev/ttyACM0',115200,timeout=0.5)

def submit(s):
    s = (s + "\n").encode('utf-8')
    ok = False
    while not ok:
        port.write(s)
        r = port.readline().decode('utf-8').strip();
        print(f"{s.strip()} -> {r}")
        ok = r == "ok"
        if not ok:
            time.sleep(0.05)
    

submit("M17")
submit("G92")
for i in range(200):
    r = 20+i
    x = math.sin(i/20) * r
    y = math.cos(i/20) * r
    submit(f"G0 X{x:.2f} Y{y:.2f} F20")
