#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void Task2(){
  for(;;){
    PORTB = 0x20;
    _delay_ms(200);
    Task_Yield();
  }
}

void Task1()
{
  for(;;){
    PORTB = 0x40;
    _delay_ms(200);
    Task_Yield();
  }
}

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  Task_Create(Task1, 0x00, 0x00);
  Task_Create(Task2, 0x00, 0x00);
  Task_Terminate();
}
