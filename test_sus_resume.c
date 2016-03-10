#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned int count = 0;

void Task_2(){
  for(;;){
    PORTB = 0x20;
  }
}

void Task_1()
{
  for(;;){
    PORTB = 0x40;
  }
}

ISR(TIMER1_COMPA_vect){
		count++;
		if(count == 6) {
			Task_Suspend(2);
		}
		if(count == 12) {
			Task_Resume(2);
			count = 0;
		}
    Task_Yield();
    PORTB = 0x80;

}

int main() {
	DDRB = 0xF0;
	PORTB = 0x00;

	OS_Init();
  Task_Create(Task_1, 0x00, 0x00);
  Task_Create(Task_2, 0x00, 0x00);

  //Clear timer config.
  TCCR1A = 0;

  //Set to CTC (mode 4)
  TCCR1B |= (1<<WGM12);

  //Set prescaler to 256
  TCCR1B |= (1<<CS12);

  //Set TOP value (0.01 seconds)
  OCR1A = 6250;
 
  //Enable interupt A for timer 3.
  TIMSK1 |= (1<<OCIE1A);
  //Set timer to 0 (optional here).
  TCNT1 = 0;

  OS_Start();
}