PROJECT = keyboard
MCU = attiny85


OBJ_PATH = obj

CC = avr-gcc
OBJCOPY = avr-objcopy
ASIZE=avr-size

FLASHER_PATH = ~/software/micronucleus/commandline/


CFLAGS = -Wall -Os -Iusbdrv -mmcu=$(MCU) -DF_CPU=16500000 -I../
CFLAGS += -I../usbdrv

VPATH = ../usbdrv

OBJFLAGS = -j .text -j .data -O ihex


PROJECT_SRC  = $(PROJECT).c
PROJECT_SRC += usbdrv.c
PROJECT_SRC += oddebug.c

PROJECT_ASM_SRC  = usbdrvasm.S

OBJECTS  = $(patsubst %.c, $(OBJ_PATH)/%.o, $(PROJECT_SRC))
OBJECTS += $(patsubst %.S, $(OBJ_PATH)/%.o, $(PROJECT_ASM_SRC))

CHEAT_ARG := $(shell mkdir -p $(OBJ_PATH))


all: $(PROJECT).hex size

size: $(PROJECT).elf
	@$(ASIZE) --mcu=$(MCU) --format=avr $(PROJECT).elf


signals.h: Makefile signals.conf
	@echo "Generate signals.h"
	@rm -f signals.h
	@echo '#ifndef SIGNALS_H\n#define SIGNALS_H\n#include <avr/io.h>\n' > signals.h
	@awk '{print "#define "$$1"_PIN_REG PIN"$$2"\n#define "$$1"_PORT_REG PORT"$$2"\n#define "$$1"_DDR_REG DDR"$$2"\n#define "$$1"_PIN (1 << P"$$2$$3")\n"}' signals.conf >> signals.h
	@echo '#endif' >> signals.h

%.hex: %.elf
	@echo "Generate $@"
	@$(OBJCOPY) $(OBJFLAGS) $< $@

$(PROJECT).elf: $(OBJECTS)
	@echo "Generate $@"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $@

$(OBJ_PATH)/%.o: %.c Makefile usbconfig.h signals.h
	@echo "Compile $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_PATH)/%.o: %.S Makefile
	@echo "Compile $<"
	@$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

upload: $(PROJECT).hex
	$(FLASHER_PATH)/micronucleus $(PROJECT).hex

## Clean target
.PHONY: clean
clean:
	@rm -rf $(TARGET).elf $(TARGET).hex $(TARGET).eep $(TARGET).cof $(TARGET).map
	@rm -rf dep/
	@rm -rf $(OBJ_PATH)/
	@rm -rf signals.h
