#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

unsigned int blinks = 1;
EVENT blink_increment;

void Task_1() {
	int i;
	while(1) {
		Event_Signal(blink_increment);

		for(i = 0; i<blinks; i++) {
			PORTB = 0x20;
		    _delay_ms(500);
		    PORTB = 0x00;
		    _delay_ms(500);
		}

		blinks++;
		Task_Yield();
	}
}

void Task_2() {
	while(1) {
		Event_Wait(blink_increment);

		int i;
		for(i = 0; i<blinks; i++) {
			PORTB = 0x10;
		    _delay_ms(500);
		    PORTB = 0x00;
		    _delay_ms(500);	
		}

		Task_Yield();
	}
}

void a_main() {
	DDRB = 0xF0;
	PORTB = 0x00;

	blink_increment = Event_Init();

	// if these tasks are swapped so task 2 must wait on task 1 system fails
	Task_Create(Task_1, 0, 0);
	Task_Create(Task_2, 0, 0);
}
