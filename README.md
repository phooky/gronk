# GRONK is a plotter.

GRONK is our large-format plotter. It has a bit more than 130cm of
travel in the X direction and unlimited travel in the Y direction.

Internally, GRONK uses the electronics from an old school MakerBot
Replicator 2 (a MightBoard rev. H). The firmware here uses a couple of
bits of the original code but is mostly new.

GRONK accepts G codes over USB.

GRONK is named after the horrible noise it made when outfitted with
full-step stepper drivers.

## Quickstart

0. Install python and pyserial on the computer you want to use.
1. Plug in GRONK (power receptacle on the right side) and turn on the
   switch next to the power cable.
2. Connect the USB cable on the left side to your computer; GRONK will
   appear as a USB serial port. Connect to it at 115200 baud.
3. Load paper and pen.
3. Send G-Code commands to GRONK by either:
    1. Connecting directly with a terminal emulator and trying out
       gcode commands manually. (Be sure to issue the M230 command to
       turn on echo mode, so you can see what you're typing, and be
       aware that the steppers will be disabled at startup and will
       need to be enabled before the steppers will move!)
    2. Use one of the python scripts in the "scripts" directory to
       send a prepared gcode file or generated commands.


## Setting up GRONK

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

### Connecting to GRONK

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

| ok   | the command has been processed and accepted.                                    |
| full | the movement queue is full and the command has been discarded; wait and resend. |
| err  | there is an error or unrecognized command.                                      |

The valid GCode commands are specified below.

**Note:** if you try to set the USB serial connection to 1200 baud, it
will reset the microcontroller and put it in programming mode. This is
a common case for many Arduino-derived projects. If you accidentally
put GRONK in programming mode, disconnect and reconnect the USB cable.

## GCode Reference

Table of supported G codes

| G-code | Meaning                      | Implemented | Tested | Notes |
|--------|------------------------------|-------------|--------|-------|
| G00    | Ordinary movement            | X           | X      |       |
| G01    | Fast travel                  | X           | X      |       |
| G04    | Dwell                        | X           |        | [^1]  |
| G92    | Set current position as zero | X           | X      |       |

[^1] P parameter is *ALWAYS* in ms

Table of supported M codes

| M-code      | Meaning                                          | Implemented | Tested |
|-------------|--------------------------------------------------|-------------|--------|
| M03         | Plotter pen down                                 | X           | X      |
| M04         | Plotter pen up                                   | X           | X      |
| M17 [X] [Y] | Enable steppers. If none specified, enable all.  | X           | X      |
| M18 [X] [Y] | Disable steppers. If none specified, enable all. | X           | X      |
| M230        | Turn on command echoing.                         | X           | X      |
| M231        | Turn off command echoing.                        | X           | X      |
| M255        | Reset machine.                                   |             |        |


## TODO

There's still a little work to do on GRONK.
* Improve maximum travel speed
* Editable default speeds for G0 and G1 commands
* Fix X axis endstop to provide zeroing
* Implement the machine reset command
* Tack on a Pico W or Pi to provide wireless streaming or store programs
* Print out new levers to replace the broken ones
