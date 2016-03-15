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

test_sus_dne: test_sus_dne.c
	$(CC) $(FLAGS) test_sus_dne.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_sus_dne.o

test_sus: test_sus_resume.c
	$(CC) $(FLAGS) test_sus_resume.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_sus_resume.o

test_sleep: test_sleep.c
	$(CC) $(FLAGS) test_sleep.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_sleep.o

test_pre_empt: test_pre_empt.c
	$(CC) $(FLAGS) test_pre_empt.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_pre_empt.o

test_mutex: test_mutex.c
	$(CC) $(FLAGS) test_mutex.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_mutex.o

test_mutex_dne: test_mutex_dne.c
	$(CC) $(FLAGS) test_mutex_dne.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_mutex_dne.o

test_event: test_event.c
	$(CC) $(FLAGS) test_event.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_event.o

test_switch: test_task_switch.c
	$(CC) $(FLAGS) test_task_switch.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_task_switch.o


test_1: test_1.c
	$(CC) $(FLAGS) test_1.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_1.o

test_2: test_2.c
	$(CC) $(FLAGS) test_2.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_2.o

test_3: test_3.c
	$(CC) $(FLAGS) test_3.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_3.o

test_4: test_4.c
	$(CC) $(FLAGS) test_4.c
	$(CC) $(ELFFLAGS) img.elf cswitch.o os.o test_4.o


test: compile test_1 hex load

clean:
	rm -f *.elf *.o *.hex
