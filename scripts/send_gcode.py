#!/usr/bin/env python

import argparse
from gronklib import Gronk

parser = argparse.ArgumentParser(
    prog = 'send_gcode.py',
    description = 'Send a GCode file to Gronk.')
parser.add_argument('-P', '--port', default='/dev/ttyACM0')
parser.add_argument('-v', '--verbose', action='store_true')
parser.add_argument('filename',metavar='FILE',nargs='+',help="A list of files to send to Gronk, or '-' to send input from stdin.")

args = parser.parse_args()

# validate args
if '-' in args.filename:
    if len(args.filename) > 1:
        print("Cannot mix stdin with other file paths.")
        sys.exit(1)


gronk = Gronk(args.port,args.verbose)
gronk.enable_steppers()
send_to_gronk('M17') # In case the user forgets
for filename in args.filename:
    gronk.send_file(filename)
gronk.wait_until_done()
gronk.disable_steppers()
