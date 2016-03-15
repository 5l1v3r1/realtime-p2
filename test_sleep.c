#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void Task_3(){
  for(;;){
    PORTB = 0x80;
    _delay_ms(50);
    Task_Yield();
  }
}

void Task_2(){
  for(;;){
    Task_Sleep(100);
    
    PORTB = 0x20;
    _delay_ms(50);
    
    Task_Yield();
  }
}

void Task_1(){
  for(;;){
    Task_Sleep(300);

    PORTB = 0x40;
    _delay_ms(50);

    Task_Yield();
  }
}

void a_main() {
	DDRB = 0xF0;
	PORTB = 0x00;

  Task_Create(Task_1, 0x00, 0x00);
  Task_Create(Task_2, 0x00, 0x00);
  Task_Create(Task_3, 0x00, 0x00);
}
