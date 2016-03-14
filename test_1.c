#include "os.h"
#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>

//
// LAB - TEST 1
//  Noah Spriggs, Murray Dunne
//
//
// EXPECTED RUNNING ORDER: P1,P2,P3,P1,P2,P3,P1
//
// P1 sleep              lock(attept)            lock
// P2      sleep                     signal
// P3           lock wait                  unlock

MUTEX mut;
EVENT evt;

void Task_P1(){
    PORTB   = 0x10;
    _delay_ms(1);
    PORTB   = 0x00;
    
    Task_Sleep(10); // sleep 100ms
    
    PORTB   = 0x10;
    _delay_ms(1);
    PORTB   = 0x00;
    
    Mutex_Lock(mut);
    for(;;);
}

void Task_P2(){
    PORTB   = 0x20;
    _delay_ms(1000);
    PORTB   = 0x00;
    
    Task_Sleep(20); // sleep 200ms
    Event_Signal(evt);
    for(;;);
}

void Task_P3(){
    PORTB   = 0x40;
    _delay_ms(1000);
    PORTB   = 0x00;

    Mutex_Lock(mut);
    Event_Wait(evt);
    Mutex_Unlock(mut);
    for(;;);
}

void a_main(){
    DDRB    = 0xF0;
    PORTB   = 0x00;

    mut = Mutex_Init();
    evt = Event_Init();

    Task_Create(Task_P1, 1, 0);
    Task_Create(Task_P2, 2, 0);
    Task_Create(Task_P3, 3, 0);
}