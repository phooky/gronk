# Gen3 -> MightBoard transition notes

## Original connections

### Relay board

A diode is run from Red to Blue (A- to A+)

| Wire | Terminal on relay board |
|------|-------------------------|
| Red  | A-                      |
| Blue | A+                      |

The A port on the relay board is connected to the B port on the motherboard.
The B port on the relay board (unused) is connected to the C port on the motherboard.

### LEDs
The A port on the motherboard is connected to a white/black wire pair that runs into 
an LED strip. These pull about 1A at 12V.

| Wire  | Terminal on motherboard |
|-------|-------------------------|
| White | A+                      |
| Black | A-                      |

### X axis endstops

These are wired through a four-conductor cable connected to the X axis daughterboard
through a mess of hot glue; I'm not going to try to unravel that. (It's Blk, R, G, Y;
R and G seem to be the signals?)

## Current status

### X and Y motors

Both are hooked up to the respective axes of the Mightyboard and operate at 24V.

### Installing latest firmware

On Linux, you'll need to have an AVR toolchain installed. On Debian and similar, you can do this
with:

```
sudo apt install gcc-avr binutils-avr avrdude avr-libc
```

To build and upload the firmware, first connect the USB cable to your computer. Then:

```
cd firmware
make
sudo make upload
```



