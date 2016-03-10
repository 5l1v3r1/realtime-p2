CC=avr-gcc
COPY=avr-objcopy
LOAD=avrdude
FLAGS=-g -Os -mmcu=atmega2560 -c
ELFFLAGS= -g -mmcu=atmega2560 -o
HEXFLAGS=-j .text -j .data -O ihex
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
LOADFLAGS= -p m2560 -c stk500v2 -P /dev/ttyACM0 -b 115200 -U flash:w:img.hex:i -V -v -D
endif
ifeq ($(UNAME), Darwin)
LOADFLAGS= -p m2560 -c stk500v2 -P /dev/cu.usbmodem1411 -b 115200 -U flash:w:img.hex:i -V -v -D
endif


all: clean compile elf hex load

compile: cswitch.S os.c
	$(CC) $(FLAGS) os.c
	$(CC) $(FLAGS) cswitch.S

elf: cswitch.o os.o
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o

hex: img.elf
	$(COPY) $(HEXFLAGS) img.elf img.hex

load:
	$(LOAD) $(LOADFLAGS)

test1: test_sus_resume.c
	$(CC) $(FLAGS) test_sus_resume.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_sus_resume.o

test: compile test1 hex load

clean:
	rm -f *.elf *.o *.hex
