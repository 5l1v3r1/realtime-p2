#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


// On interrupt switch task
ISR(TIMER1_COMPA_vect){
    if(isActive()){
    Task_Yield();
    PORTB = 0x80;
  }
}

void Task2(){
  for(;;){
    PORTB = 0x20;
  }
}

void Task1()
{
  for(;;){
    PORTB = 0x40;
  }
}

void main() 
{
  DDRB = 0xF0;
  PORTB = 0x00;

  OS_Init();
  Task_Create(Task1, 0x00, 0x00);
  Task_Create(Task2, 0x00, 0x00);

  //Clear timer config.
  TCCR1A = 0;

  //Set to CTC (mode 4)
  TCCR1B |= (1<<WGM12);

  //Set prescaler to 256
  TCCR1B |= (1<<CS12);

  //Set TOP value (0.01 seconds)
  OCR1A = 6250;
 
  //Enable interupt A for timer 3.
  TIMSK1 |= (1<<OCIE1A);
  //Set timer to 0 (optional here).
  TCNT1 = 0;

  OS_Start();

  while(1);
}