PROJECT = keyboard

CC = avr-gcc
OBJCOPY = avr-objcopy

VPATH += ../

CFLAGS = -Wall -Os -Iusbdrv -mmcu=attiny85 -DF_CPU=16500000 -I../
OBJFLAGS = -j .text -j .data -O ihex

OBJECTS = usbdrv/usbdrv.o usbdrv/oddebug.o usbdrv/usbdrvasm.o $(PROJECT).o

all: $(PROJECT).hex

clean:
	$(RM) *.o *.hex *.elf usbdrv/*.o

%.hex: %.elf
	$(OBJCOPY) $(OBJFLAGS) $< $@

$(PROJECT).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(OBJECTS): usbdrv/usbconfig.h

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S Makefile
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

upload: $(PROJECT).hex
	micronucleus $(PROJECT).hex

test:
	cat $(VPATH)
