#!/usr/bin/env python

from multiprocessing.connection import Listener


address = ("localhost", 6543)
listener = Listener(address, authkey = b"gronk")
curfile = None
while True:
    c = listener.accept()
    try:
        r = c.recv()
        print("RECEIVED:",r)
        if r == "Q": # query current file
            c.send(curfile)
        elif r == "stat_req":
            c.send({"okay":"doing fine"})
        else:
            curfile = r
    except Exception as e:
        print("Exception ",e)
        

    
