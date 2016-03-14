/* Test Preemetive scheduling as well as the Task_GetArg();
 *
 */
#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#include <util/delay.h>
extern void debug_flash();

void Task_1(){
  int x = Task_GetArg();
  if(x == 1){
    PORTB = 0x10;
    _delay_ms(500);
  }
}

void Task_2(){
  int task_1_id = Task_Create(Task_1, 1, 1);
  int x = Task_GetArg();
  if(x == 2){
    PORTB = 0x20;
    _delay_ms(500);
  }
}

void Task_3(){
  int task_2_id = Task_Create(Task_2, 2, 2);
  int x = Task_GetArg();
  if(x == 3){
    PORTB = 0x40;
    _delay_ms(500);
  }
}

void Task_4(){
  int task_3_id = Task_Create(Task_3, 3, 3);
  int x = Task_GetArg();
  if(x == 4){
    PORTB = 0x80;
    _delay_ms(500);
  }
}

void Task_5(){
  int task_4_id = Task_Create(Task_4, 4, 4);
  int x = Task_GetArg();
  if(x == 5){
    PORTB = 0xF0;
    _delay_ms(500);
  }
}


int a_main() {
  DDRB = 0xF0;
  PORTB = 0x00;

  int task_5_id = Task_Create(Task_5, 5, 5); 
}
