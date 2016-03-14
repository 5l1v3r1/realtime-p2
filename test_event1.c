/*** Tests tasks calling event signal and wait  ***/

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

unsigned int blinks = 1;
EVENT blink_increment;

void Task_1() {
	blink_increment = Event_Init();
	int i;
	while(1) {
		Event_Signal(blink_increment);
		for(i = 0; i<blinks; i++) {
			PORTB = 0x20;
		    _delay_ms(500);
		    PORTB = 0x00;
		    _delay_ms(500);
		}
		++blinks;
		Task_Yield();
	}
}

void Task_2() {
	int i;
	while(1) {
		Event_Wait(blink_increment);
		for(i = 0; i<blinks; i++) {
			PORTB = 0x10;
		    _delay_ms(500);
		    PORTB = 0x00;
		    _delay_ms(500);	
		}
	}
}

int main() {
	DDRB = 0xF0;
	PORTB = 0x00;

	OS_Init();
	Task_Create(Task_1, 0x00, 0x00);
	Task_Create(Task_2, 0x00, 0x00);
	OS_Start();
}