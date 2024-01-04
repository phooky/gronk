# GRONK is a plotter.

GRONK is our large-format plotter. It has a bit more than 130cm of
travel in the X direction and unlimited travel in the Y direction.

Internally, GRONK uses the electronics from an old school MakerBot
Replicator 2 (a MightBoard rev. H). The firmware here uses a couple of
bits of the original code but is mostly new.

GRONK accepts gcode over USB. You can use `vpype-gcode` to convert SVG
files to appropriate gcode; see the section on [plotting
SVGs](#plot-svg).


GRONK is named after the horrible noise it made when outfitted with
full-step stepper drivers.

## Quickstart

0. Install python and pyserial on the computer you want to use.
1. Plug in GRONK (power receptacle on the right side) and turn on the
   switch next to the power cable.
2. Connect the USB cable on the left side to your computer; GRONK will
   appear as a USB serial port. Connect to it at 115200 baud.
3. Load paper and pen.
4. Use the `send_gcode.py` script to send gcode to the plotter. You
   can either [send your own custom gcode](#sending-gcode), or
   [convert an SVG file](#plot-svg).
   
Details below.

## Setting up GRONK

### Requirements

You'll need a computer or other device that can talk to a USB serial
device. If you plan to use the included python scripts, make sure you
have installed python and the pyserial library. (On a Debian-derived
box you may want to install pyserial system-wide, if it's not there
already, with `apt install python3-serial`; on all other systems if
pip is installed you can just do `pip install pyserial`.)

### Powering up

On the far left of the machine is a receptacle for an IEC power
cable. (If you don't know what an IEC cable is, take a look at the
socket and you'll probably recognize it by sight.) There should be a
cable that is stored with GRONK that you can use. Put GRONK on a flat
surface, plug it in, and turn on the power switch right next to the
cable.

### Loading up paper

On the back of the machine you'll see several plastic levers. These
raise and lower the rollers that hold the paper in the machine. They
are adjustable and can be moved along the length of the machine to
provide a better grip on different paper sizes. Raise all the rollers,
insert and align your paper, and then lower them. **Note:** some of
the levers have broken off. You can still raise and lower them by
carefully rotating the parts they were attached to.

### Loading a pen

To be documented. We need to measure the dimensions and suggested
stick-out of the pen holder!

### Connecting your computer to GRONK

GRONK uses a USB serial connection clocked at 115200 baud. To connect,
find the USB cable that emerges from the machine on the *right* side
(opposite the power cable) and plug it in to your computer. A serial
port should become available. How this appears and what tools you use
to communicate will vary from platform to platform; on most Linux
machines it will show up as /dev/ttyACM0.

You can communicate with GRONK using the serial communications
software of your choice, or the python script included in this
repository.

All commands to GRONK should be terminated with a newline. GRONK will
immediately return a newline-terminated string that begins with one of
the following:

| code | meaning                                                                         |
|------|---------------------------------------------------------------------------------|
| ok   | the command has been processed and accepted.                                    |
| full | the movement queue is full and the command has been discarded; wait and resend. |
| err  | there is an error or unrecognized command.                                      |

The valid GCode commands are specified below.

**Note:** if you try to set the USB serial connection to 1200 baud, it
will reset the microcontroller and put it in programming mode. This is
a common case for many Arduino-derived projects. If you accidentally
put GRONK in programming mode, disconnect and reconnect the USB cable.

### GRONK's default state

When GRONK turns on, it should default to:

| Setting  | Value    |
|----------|----------|
| Echo     | OFF      |
| Steppers | DISABLED |
| Position | 0,0      |

You will need to issue an M17 command to turn on the steppers before
anything will move.

## GCode Reference

GCode is a very old format for sending commands to CNC machines. Each
line consists of a G-code or M-code command, followed by a number of
parameter codes. The X and Y parameters are always in mm. The
feedrate, F, is in mm/s.

G-codes usually pertain to movement, and M-codes generally are for
everything else.

| G-code | Meaning                      | Example         | Explanation                               |
|--------|------------------------------|-----------------|-------------------------------------------|
| G00    | Fast travel                  | G00 X10 Y20 F30 | Move to (10,20) at a speed of 30mm/s [^1] |
| G01    | Ordinary movement            | G01 X10 Y20 F30 | Same as above [^1]                        |
| G04    | Dwell                        | G04 P20         | Pause for 20ms [^2]                       |
| G92    | Set current position as zero | G92             | Set current location as (0,0)             |


[^1] At present, the feedrate parameter is always required. In the
future there will be reasonable defaults for G0 and G1; for now
they're identical.
[^2] P parameter is *ALWAYS* in ms

Table of supported M codes

| M-code      | Meaning                                          |
|-------------|--------------------------------------------------|
| M03         | Plotter pen down                                 |
| M04         | Plotter pen up                                   |
| M17 [X] [Y] | Enable steppers. If none specified, enable all.  |
| M18 [X] [Y] | Disable steppers. If none specified, enable all. |
| M230        | Turn on command echoing.                         |
| M231        | Turn off command echoing.                        |
| M255        | Reset machine. [^3]                              |

[^3] Not yet implemented.

## Mounting pens

The mounting block which holds pens will securely hold a
straight-sided 12mm diameter cylinder. If you're planning on using any
kind of pen or other implement that doesn't meet that spec, you're
going to have to contruct a pen holder. A few examples are included in
the `mechanical` folder.

The pen tip should be about 30 mm below the top of the mounting
block. The block is spring-loaded, but the tip will most likely impact
the paper pretty rapidly, so if the pen tip is delicate (say, a
mechanical drafting pen) I'd recommend building some sort of
protection into your pen holder.


## Sending gcode to the printer {#sending-gcode}

You can send G-Code commands to GRONK using the `send_gcode.py` script
in the `scripts` directory. You'll need to provide the name of the
file you want to print and, depending on the platform, the name of the
port that GRONK appeared as. For example, you can run the `star.gcode` 
script on Linux with the command line `./send_gcode.py -P /dev/ttyACM0
star.gcode`.

### Usage
```
usage: send_gcode.py [-h] [-P PORT] [-v] FILE [FILE ...]

Send a GCode file to Gronk.

positional arguments:
  FILE                  A list of files to send to Gronk, or '-' to send input from stdin.

options:
  -h, --help            show this help message and exit
  -P PORT, --port PORT
  -v, --verbose
```

## Plotting SVGs {#plot-svg}

You can convert SVGs to the proper gcode format using the `vpype` and
`vpype-gcode` tools. If you've got python installed, you can install
these easily with pip:
```
pip install vpype vpype-gcode
```

There is a `vpype-gcode.toml` file in the scripts directory. You can
then use vpype to generate the correct gcode like so:

```
vpype -c vpype-gcode.toml read --quantization 0.5mm YOUR_SVG.svg gwrite YOUR_GCODE.gcode
```

Or you can just pipe the output directly to `send_gcode` without
writing out the intermediate file:

```
vpype -c vpype-gcode.toml read --quantization 0.5mm YOUR_SVG.svg gwrite - | ./send_gcode.py -
```

## TODO

There's still a little work to do on GRONK.
* Editable default speeds for G0 and G1 commands
* Fix X axis endstop to provide zeroing
* Implement the machine reset command
* Tack on a Pico W or Pi to provide wireless streaming or store programs
