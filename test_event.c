/* Tests a task waiting on an event before the signal is sent */

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

unsigned int blinks = 1;
EVENT blink_increment;

extern void debug_flash();

void debug_flash_2(){
	PORTB = 0xF0;
	_delay_ms(500);
	PORTB = 0x00;
	_delay_ms(500);
};


void Task_2();

void Task_1() {
	// Flashes all LEDS should happen before debug_flash()
	debug_flash_2();

	//4. Signal to Task_2 waiting on blink_increment
	Event_Signal(blink_increment);

	int i;
	for(i = 0; i<blinks; i++) {
		PORTB = 0x20;
	    _delay_ms(500);
	    PORTB = 0x00;
	    _delay_ms(500);
	}

	blinks++;

	Task_Create(Task_2, 0, 0);
	
	Task_Terminate();
}

void Task_2() {
	//2. Create Task_1 which will signal Task_2
	Task_Create(Task_1, 1, 0);

	//3. Wait for signal, this will yield to the lower priority task_1
	Event_Wait(blink_increment);

	//Flashes the two leds at pin 11 & 12 should occur after debug_flash_2();
	debug_flash();

	int i;
	for(i = 0; i<blinks; i++) {
		PORTB = 0x10;
	    _delay_ms(500);
	    PORTB = 0x00;
	    _delay_ms(500);	
	}

	Task_Terminate();
}

void a_main() {
	DDRB = 0xF0;
	PORTB = 0x00;

	blink_increment = Event_Init();

	//1. Start Task_2
	Task_Create(Task_2, 0, 0);
}
