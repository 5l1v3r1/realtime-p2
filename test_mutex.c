#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void Task_2(){
  MUTEX m = Mutex_Init();
  PORTB = 0x20;
  _delay_ms(500);
  PORTB = 0x00;
}

void Task_1(){
  MUTEX m = Mutex_Init();
  PORTB = 0x10;
  _delay_ms(500);
  if(m == 0){
    Task_Create(Task_2, 0, 0); // Pin 12
  }
}

int main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  OS_Init();
  Task_Create(Task_1, 0, 0); // Pin 12
  OS_Start();
}
