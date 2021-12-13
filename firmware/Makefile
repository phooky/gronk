
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
	LiquidCrystalSerial.o ButtonArray.o Steppers.o \
	UART.o CommandParser.o

%.o: %.cc
	$(CPP) -c -o $@ $(CFLAGS) $<

$(TARGET_HEX): $(TARGET_ELF)
	avr-objcopy -O ihex -R .eeprom $< $@

$(TARGET_ELF): $(OBJS)
	/usr/bin/avr-gcc -mmcu=$(CPU) -Os -Wl,-Map,$(TARGET).map -Wl,--gc-sections -o $@ $^ -lm

upload: $(TARGET_HEX)
	avrdude -F -p m1280 -P $(PORT) -c stk500v1 -b 57600 -U flash:w:$(TARGET_HEX)

clean:
	rm $(OBJS) $(TARGET_HEX) $(TARGET_ELF) $(TARGET_MAP)
