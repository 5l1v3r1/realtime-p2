#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void Task_2();
MUTEX shared_mutex;

void Task_1(){
  //12. Finally run Task_1's blink LED pin 11 
  PORTB = 0x20;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //13. Create Task_2 so loop continues at step 1 again.
  Task_Create(Task_2, 3, 0);
}

void Task_3(){
  //7. Lock locked mutex, should block Task 3 and Task 2 should inherit priority
  //   after blocking on mutex lock, dispatches to next highest priority task
  //   which will now be Task_2 since it has inherited Task_3's priority
  Mutex_Lock(shared_mutex);

  //10. Once Mutex has been released blink the LED on pin 10
  PORTB = 0x10;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //11. Unlock the Mutex for completeness
  Mutex_Unlock(shared_mutex);
}

void Task_2(){
  //3. Lock the shared mutex
  Mutex_Lock(shared_mutex);

  //4. Create Task_3 with higher priority than Task_2
  int Task_3_id = Task_Create(Task_3, 1, 0);

  //5. Create Task_2 with higher priority than Task_2 but lower Priority than Task_3
  int Task_2_id = Task_Create(Task_1, 2, 0);
  
  //6. Yield to Task_3
  Task_Yield();

  //8. Return from Task_3 here and blink pin 13 LED
  PORTB = 0x80;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //9. Unlock the Mutex ->  unblocking Task_3
  Mutex_Unlock(shared_mutex);
}


// OLD BUG but good explanation why it is bad to use the number of tasks as the ID
// after this terminates any new tasks will have overlapping id's
// example
// a_main == id == tasks == 0
// task_2_id == tasks == 1
// Terminate a_main -> tasks = tasks - 1
// task_3_id == tasks == 1

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  // 1. Create the shared mutex
  shared_mutex = Mutex_Init();

  // 2. Create the first task to run aka Task_2, with the lowest priority
  Task_Create(Task_2, 3, 0);
}
