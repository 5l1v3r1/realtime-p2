#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>

MUTEX shared_mutex;

void Task_2(){
  Mutex_Lock(shared_mutex);
  Task_Yield();
  PORTB = 0x20;
  _delay_ms(1000);
  PORTB = 0x00;
  _delay_ms(1000);
  Mutex_Unlock(shared_mutex);
}

void Task_1(){
  _delay_ms(5000);
  Mutex_Lock(shared_mutex);
  Mutex_Unlock(shared_mutex);
}

int main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  OS_Init();
  shared_mutex = Mutex_Init();
  Task_Create(Task_1, 1, 0); // Pin 12
  Task_Create(Task_2, 0, 0); // Pin 12
  OS_Start();
}
