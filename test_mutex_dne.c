/* Tests error handling of mutex lock and unlock */

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

//MUTEX shared_mutex;

void Task_1(){

  Mutex_Lock(8);

  PORTB = 0x20;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  Mutex_Unlock(8);

  Mutex_Lock(4);

  PORTB = 0x20;
  _delay_ms(200);
  PORTB = 0x00;
  _delay_ms(200);

  Mutex_Unlock(4);

}

void a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;
  Task_Create(Task_1, 0, 0);
}
