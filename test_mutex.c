#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

MUTEX shared_mutex;

void Task_1(){
  PORTB = 0x20;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);
  Task_Yield();
}

void Task_3(){
  //Lock locked mutex, should block Task 3 and pass priority to Task 2
  Mutex_Lock(shared_mutex);
  PORTB = 0x10;
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
  int Task_3_id = Task_Create(Task_3, 1, 0); // Pin 12
  //Yield to task 3
  Task_Yield();

  //Create Task_2 
  int Task_2_id = Task_Create(Task_1, 2, 0); // Pin 12
  //Task should not yield to task 2 since it has inherited a higher priority from task 3
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

// after this terminates any new tasks will have overlapping id's
// example
// a_main == id == tasks == 0
// task_2_id == tasks == 1
// Terminate a_main -> tasks = tasks - 1
// task_3_id == tasks == 1

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  shared_mutex = Mutex_Init();
  Task_Create(Task_2, 3, 0);
}
