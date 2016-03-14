#include "os.h"
#include <avr/io.h>
//
// LAB - TEST 2
//  Noah Spriggs, Murray Dunne, Daniel McIlvaney

// EXPECTED RUNNING ORDER: P1,P2,P3,P1,P2,P3,P1
//
// P1 sleep              lock(attept)            lock
// P2      sleep                     resume
// P3           lock suspend               unlock

MUTEX mut;
volatile PID pid;

void Task_P1(){
    Task_Sleep(10); // sleep 100ms
    Mutex_Lock(mut);
    for(;;);
}

void Task_P2(){
    Task_Sleep(20); // sleep 200ms
    Task_Resume(pid);
    for(;;);
}

void Task_P3(){
    Mutex_Lock(mut);
    Task_Suspend(pid);
    Mutex_Unlock(mut);
    for(;;);
}

void a_main(){
    mut = Mutex_Init();
    evt = Event_Init();

    Task_Create(Task_P1, 1, 0);
    Task_Create(Task_P2, 2, 0);
    pid = Task_Create(Task_P3, 3, 0);
}
