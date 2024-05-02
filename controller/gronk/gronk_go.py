#!/usr/bin/env python

import RPi.GPIO as GPIO
import time
from enum import Enum
from gronklib import Gronk
from gronkbuttons import Buttons
from gronkdisplay import Display
from queue import Queue
from threading import Thread, Event, Timer
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

display = Display()


# Todo -- status should return pen state!
pendown = None # pen starts in indeterminate state; assume down

display.show_ready()

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
        try:
            cur_file = c.recv()
            if c == "":
                return
            elif c == "Q":
                c.send(cur_file)
            else:
                display.show_ready(cur_file[0])
        except Exception as e:
            print("Exception ",e)

ft = Thread(target=file_thread)
ft.start()

def shutdown():
    global full_shutdown
    print("SHUTDOWN NOW")
    full_shutdown = True
    running.clear()
    # handle shutting down listener thread
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
    global shutdown_timer
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

if full_shutdown:
    print("Full shutdown")
    display.show_goodbye()
    time.sleep(2);
    subprocess.Popen(['sudo','shutdown','-h','now'])

GPIO.cleanup()


