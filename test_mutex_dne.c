/* Tests error handling of mutex lock and unlock */

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

void Task_2();

void Task_1(){
  //2. Lock Mutex 0 which does not exist -> NoOp
  Mutex_Lock(0);

  //3. Yield to Task_2
  Task_Yield();

  //6. Run the blink after Task_2 completes
  PORTB = 0x20;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //7. Perform NoOp
  Mutex_Unlock(0);
  
  //8. Loop
  Task_Create(Task_1, 0, 0);
  Task_Create(Task_2, 0, 0);
}

void Task_2(){
  //3. Call Lock on mutex that DNE should NoOP instead of blocking
  Mutex_Lock(0);

  //4. Should blink next since not blocked by non-existant Mutex
  PORTB = 0x10;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  //5. Perform NoOp
  Mutex_Unlock(0);
}

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  //1. Create Task_1 & Task_2
  Task_Create(Task_1, 0, 0);
  Task_Create(Task_2, 0, 0);
}
