#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

MUTEX shared_mutex;

void Task_1(){
  PORTB = 0x10;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);
  Task_Yield();
}

void Task_3(){
  //Lock locked mutex, should block Task 3 and pass priority to Task 2
  Mutex_Lock(shared_mutex);
  PORTB = 0x40;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);
  Mutex_Unlock(shared_mutex);
  Task_Yield();
}

void Task_2(){
  //Lock the shared mutex, ensures that task_2 runs first due to priority inheritence
  Mutex_Lock(shared_mutex);

  //Create Task_3 
  Task_Create(Task_3, 1, 0); // Pin 12
  //Yield to task 3
  Task_Yield();

  //Create Task_1
  Task_Create(Task_1, 2, 0); // Pin 12
  //Yield to highest priority task, should be task 1 since it inherits Task_3's priority
  Task_Yield();

  PORTB = 0x80;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //Unlock the mutex
  Mutex_Unlock(shared_mutex);
  
  //Yield should send to task 3
  Task_Yield();
}

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  shared_mutex = Mutex_Init();
  Task_Create(Task_2, 3, 0); // Pin 12
}
