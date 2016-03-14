/*** Tests suspending and resuming a task from another task **/

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

int Task_1_id;
int Task_2_id;

void Task_2(){
  for(;;){
    PORTB = 0x10;
    _delay_ms(100);
    PORTB = 0x00;
    _delay_ms(100);
    Task_Yield();
  }
}

void Task_1(){
  volatile unsigned int count = 0;

  while(1){
    count++;
    PORTB = 0x80;
    _delay_ms(100);
    PORTB = 0x00;
    _delay_ms(100);

    if(!(count%8)){
      Task_Suspend(Task_2_id);
    }else if(!(count%4)){
      Task_Resume(Task_2_id);
    }

    Task_Yield();
  }
}

void a_main() {
	DDRB = 0xF0;
	PORTB = 0x00;

  Task_1_id = Task_Create(Task_1, 0x00, 0x00);
  Task_2_id = Task_Create(Task_2, 0x00, 0x00);
}
