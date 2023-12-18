#!/usr/bin/env python

import serial
import argparse
import time

parser = argparse.ArgumentParser(
    prog = 'send_gcode.py',
    description = 'Send a GCode file to Gronk.')
parser.add_argument('-P', '--port', default='/dev/ttyACM0')
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('filename',metavar='FILE',nargs='+')

args = parser.parse_args()
s = serial.Serial(args.port,115200,timeout = 0.25)
# clear out data
print("connecting.")
while len(s.read(100)) > 90:
    pass
tries = 0

while True:
    tries += 1
    s.write(b"M231\n")
    if s.readline().strip() == b'ok':
        break
    if s.readline().strip() == b'ok':
        break
    if tries > 3:
        print("Still trying to connect...")

class GCodeError(Exception):
    pass

lineno = 0

def send_to_gronk(line):
    global lineno
    lineno += 1
    delay = False
    while True:
        data = line.strip().encode('ascii')+b'\n'
        s.write(data)
        rsp = s.readline().strip()
        if rsp == b'ok':
            if delay and args.verbose:
                print("resent.")
            return
        elif rsp == b'full':
            if not delay and args.verbose:
                print("full, trying resend...")
                delay = True
            time.sleep(0.05)
        elif rsp[:3] == b'err':
            raise GCodeError(line + "(" + rsp.decode('utf-8')+")")
        else:
            raise Exception("Bad response "+rsp.decode('utf-8')+" (to "+line+")")

for filename in args.filename:
    f = open(filename)
    for line in f.readlines():
        send_to_gronk(line)

s.flush()
