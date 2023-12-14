
CPU=atmega1280
CFLAGS=-DF_CPU=16000000L -DREVG -DMODEL_REPLICATOR2 -DSPECIFIC_REP2 \
-mmcu=$(CPU) -g -Os -Wall -Winline -fno-exceptions -ffunction-sections \
-fdata-sections -std=c++11
CPP=/usr/bin/avr-g++

PORT ?= /dev/ttyACM0
TARGET=rep2plot
TARGET_ELF=$(TARGET).elf
TARGET_HEX=$(TARGET).hex
TARGET_MAP=$(TARGET).map

OBJS=Main.o Pin.o SoftI2cManager.o TWI.o RGB_LED.o \
	LiquidCrystalSerial.o ButtonArray.o Motion.o \
	UART.o CommandParser.o

%.o: %.cc
	$(CPP) -c -o $@ $(CFLAGS) $<

$(TARGET_HEX): $(TARGET_ELF)
	avr-objcopy -O ihex -R .eeprom $< $@

$(TARGET_ELF): $(OBJS)
	/usr/bin/avr-gcc -mmcu=$(CPU) -Os -Wl,-Map,$(TARGET).map -Wl,--gc-sections -o $@ $^ -lm

# if you need floats in printf
#	/usr/bin/avr-gcc -mmcu=$(CPU) -Os -Wl,-u,vfprintf,-Map,$(TARGET).map -Wl,--gc-sections -o $@ $^ -lm -lprintf_flt

upload: $(TARGET_HEX)
	avrdude -F -p m1280 -P $(PORT) -c stk500v1 -b 57600 -U flash:w:$(TARGET_HEX)

clean:
	rm $(OBJS) $(TARGET_HEX) $(TARGET_ELF) $(TARGET_MAP)

reset:
	avrdude -p m1280 -P /dev/ttyACM0 -c stk500v1 -b 57600

# local test program
test: parser_test/test.cc CommandParser.cc
	g++ -o test -I . parser_test/test.cc CommandParser.cc

# DO NOT DELETE

ButtonArray.o: ButtonArray.hh Configuration.hh Fixed32.hh
ButtonArray.o: /usr/include/stdint.h AvrPort.hh Pin.hh
CommandParser.o: CommandParser.hh /usr/include/stdint.h UART.hh
LiquidCrystalSerial.o: LiquidCrystalSerial.hh Configuration.hh Fixed32.hh
LiquidCrystalSerial.o: /usr/include/stdint.h AvrPort.hh Pin.hh
LiquidCrystalSerial.o: /usr/include/stdio.h /usr/include/string.h
Main.o: ButtonArray.hh CommandParser.hh /usr/include/stdint.h
Main.o: Configuration.hh Fixed32.hh AvrPort.hh LiquidCrystalSerial.hh Pin.hh
Main.o: RGB_LED.hh Types.hh SoftI2cManager.hh Motion.hh /usr/include/stdlib.h
Main.o: UART.hh
Motion.o: Motion.hh Configuration.hh Fixed32.hh /usr/include/stdint.h
Motion.o: AvrPort.hh Pin.hh Types.hh /usr/include/stdlib.h CBuf.hh
Motion.o: SoftI2cManager.hh /usr/include/math.h UART.hh /usr/include/stdio.h
Pin.o: Pin.hh AvrPort.hh /usr/include/stdint.h
PSU.o: PSU.hh Pin.hh AvrPort.hh /usr/include/stdint.h
RGB_LED.o: RGB_LED.hh Types.hh /usr/include/stdint.h Configuration.hh
RGB_LED.o: Fixed32.hh AvrPort.hh Pin.hh TWI.hh
SoftI2cManager.o: SoftI2cManager.hh Configuration.hh Fixed32.hh
SoftI2cManager.o: /usr/include/stdint.h AvrPort.hh Pin.hh
UART.o: UART.hh /usr/include/stdint.h CBuf.hh Pin.hh AvrPort.hh
parser_test/test.o: UART.hh /usr/include/stdint.h CommandParser.hh
