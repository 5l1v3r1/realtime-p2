#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void Task_4(){
  int x = Task_GetArg();
  if(x == 0){
    PORTB = 0x80;
    _delay_ms(500);
  }
}

void Task_3(){
  int x = Task_GetArg();
  if(x == 1){
    PORTB = 0x40;
    _delay_ms(500);
  }
}

void Task_2(){
  int x = Task_GetArg();
  if(x == 2){
    PORTB = 0x20;
    _delay_ms(500);
  }
}

void Task_1(){
  int x = Task_GetArg();
  if(x == 3){
    PORTB = 0x10;
    _delay_ms(500);
  }
}

int a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  Task_Create(Task_4, 0, 0); // Pin 12
  Task_Create(Task_3, 1, 1); // Pin 12
  Task_Create(Task_2, 2, 2); // Pin 12
  Task_Create(Task_1, 3, 3); // Pin 12
}
