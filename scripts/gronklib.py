#!/usr/bin/env python

import serial
import time
import sys

class GCodeError(Exception):
    pass

class Gronk:
    def __init__(self,port,verbose = False):
        self.verbose = verbose
        self.s = serial.Serial(port,115200,timeout = 0.25)
        # clear out data
        print("connecting.")
        while len(self.s.read(100)) > 90:
            pass
        tries = 0
        while True:
            tries += 1
            self.s.write(b"M231\n")
            if self.s.readline().strip() == b'ok':
                break
            if self.s.readline().strip() == b'ok':
                break
        if tries > 3:
            print("Still trying to connect...")
        self.lineno = 0

    def send(self,line):
        self.lineno += 1
        delay = False
        while True:
            data = line.strip().encode('ascii')+b'\n'
            self.s.write(data)
            rsp = self.s.readline().strip()
            if rsp == b'ok':
                if delay and self.verbose:
                    print("resent.")
                return
            elif rsp == b'full':
                if not delay and self.verbose:
                    print("full, trying resend...")
                delay = True
                #time.sleep(0.05a)
            elif rsp[:3] == b'err':
                raise GCodeError(line + "(" + rsp.decode('utf-8')+")")
            else:
                raise Exception("Bad response "+rsp.decode('utf-8')+" (to "+line+")")

    def wait_until_done(self):
        self.send("M115")
        
    def send_file(self,path):
        self.enable_steppers()
        f = open(path) if filename != '-' else sys.stdin # need to test
        for line in f.readlines():            
            self.send(line)
        self.s.flush()

    def enable_steppers(self,enable=True):
        if enable:
            self.send('M17')
        else:
            self.send('M18')
    
    def jog(self,xdir,ydir):
        self.send(f'G100 X{xdir} Y{ydir}')

if __name__ == "__main__":
    g = Gronk("/dev/ttyACM0")
    g.send("M17")
    g.send("G0 X200 Y200")
    g.send("G0 X0 Y0")
    g.wait_until_done()
    g.send("M18")
    
