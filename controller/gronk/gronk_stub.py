#!/usr/bin/env python

from multiprocessing.connection import Listener


address = ("localhost", 6543)
listener = Listener(address, authkey = b"gronk")

while True:
    c = listener.accept()
    try:
        r = c.recv()
        print("RECEIVED:",r)
        if r == "Q": # query current file
            c.send(("name","path"))
        elif r == "stat_req":
            c.send({"okay":"doing fine"})
    except Exception as e:
        print("Exception ",e)
        

    
