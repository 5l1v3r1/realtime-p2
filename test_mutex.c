#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

MUTEX shared_mutex;

void Task_1(){
  for(;;){
    PORTB = 0x10;
    _delay_ms(200);
    PORTB = 0x00;
    _delay_ms(200);
  }
}

void Task_3(){
  //Lock locked mutex, should block Task 3 and pass priority to Task 2
  Mutex_Lock(shared_mutex);
  PORTB = 0x40;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);
}

void Task_2(){
  //Lock the shared mutex
  Mutex_Lock(shared_mutex);

  //Create Task_3 
  Task_Create(Task_3, 1, 0); // Pin 12
  //Yield to task 3
  Task_Yield();

  //Create Task_1
  Task_Create(Task_1, 2, 0); // Pin 12
  //Yield to highest priority task, should be task 1 since it inherits Task_3's priority
  Task_Yield();

  for(;;){
    PORTB = 0x80;
    _delay_ms(200);
    PORTB = 0x00;
    _delay_ms(200);
  }
}

int main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  OS_Init();
  shared_mutex = Mutex_Init();
  Task_Create(Task_2, 3, 0); // Pin 12
  OS_Start();
}
