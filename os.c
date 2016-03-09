#include <string.h>
#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Comment out the following line to remove debugging code from compiled version.
#define DEBUG

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */ 

#define WORKSPACE     256
#define MAXPROCESS   4

/*===========
  * RTOS Internal
  *===========
  */

/**
  * This internal kernel function is the context switching mechanism.
  * It is done in a "funny" way in that it consists two halves: the top half
  * is called "Exit_Kernel()", and the bottom half is called "Enter_Kernel()".
  * When kernel calls this function, it starts the top half (i.e., exit). Right in
  * the middle, "Cp" is activated; as a result, Cp is running and the kernel is
  * suspended in the middle of this function. When Cp makes a system call,
  * it enters the kernel via the Enter_Kernel() software interrupt into
  * the middle of this function, where the kernel was suspended.
  * After executing the bottom half, the context of Cp is saved and the context
  * of the kernel is restore. Hence, when this function returns, kernel is active
  * again, but Cp is not running any more. 
  * (See file "switch.S" for details.)
  */
extern void CSwitch();
extern void Exit_Kernel();    /* this is the same as CSwitch() */

/* Prototype */
void Task_Terminate(void);

/** 
  * This external function could be implemented in two ways:
  *  1) as an external function call, which is called by Kernel API call stubs;
  *  2) as an inline macro which maps the call into a "software interrupt";
  *       as for the AVR processor, we could use the external interrupt feature,
  *       i.e., INT0 pin.
  *  Note: Interrupts are assumed to be disabled upon calling Enter_Kernel().
  *     This is the case if it is implemented by software interrupt. However,
  *     as an external function call, it must be done explicitly. When Enter_Kernel()
  *     returns, then interrupts will be re-enabled by Enter_Kernel().
  */ 
extern void Enter_Kernel();

#define Disable_Interrupt()		asm volatile ("cli"::)
#define Enable_Interrupt()		asm volatile ("sei"::)
  

/**
  *  This is the set of states that a task can be in at any given time.
  */
typedef enum process_states 
{ 
   DEAD = 0, 
   READY, 
   RUNNING,
} PROCESS_STATES;

/**
  * This is the set of kernel requests, i.e., a request code for each system call.
  */
typedef enum kernel_request_type 
{
   NONE = 0,
   CREATE,
   NEXT,
   TERMINATE,
   SUSPEND
} KERNEL_REQUEST_TYPE;

/**
  * Each task is represented by a process descriptor, which contains all
  * relevant information about this task. For convenience, we also store
  * the task's stack, i.e., its workspace, in here.
  */
typedef struct ProcessDescriptor 
{
   unsigned char *sp;   /* stack pointer into the "workSpace" */
   unsigned char workSpace[WORKSPACE]; 
   PROCESS_STATES state;
   
   PID id;
   PRIORITY priority;
   int argument;
   int sus = 0;
   voidfuncptr  code;   /* function to be executed as a task */
   KERNEL_REQUEST_TYPE request;
} PD;

/**
  * This table contains ALL process descriptors. It doesn't matter what
  * state a task is in.
  */
static PD Process[MAXPROCESS];

/**
  * The process descriptor of the currently RUNNING task.
  */
volatile static PD* Cp; 

/** 
  * Since this is a "full-served" model, the kernel is executing using its own
  * stack. We can allocate a new workspace for this kernel stack, or we can
  * use the stack of the "main()" function, i.e., the initial C runtime stack.
  * (Note: This and the following stack pointers are used primarily by the
  *   context switching code, i.e., CSwitch(), which is written in assembly
  *   language.)
  */         
volatile unsigned char *KernelSp;

/**
  * This is a "shadow" copy of the stack pointer of "Cp", the currently
  * running task. During context switching, we need to save and restore
  * it into the appropriate process descriptor.
  */
volatile unsigned char *CurrentSp;

/** index to next task to run */
volatile static unsigned int NextP;  

/** 1 if kernel has been started; 0 otherwise. */
volatile static unsigned int KernelActive;  

/** number of tasks created so far */
volatile static unsigned int Tasks;  

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At( PD *p, void (*f)(void), PRIORITY py, int arg) 
{   
   unsigned char *sp;

#ifdef DEBUG
   int counter = 0;
#endif

   //Changed -2 to -1 to fix off by one error.
   sp = (unsigned char *) &(p->workSpace[WORKSPACE-1]);



   /*----BEGIN of NEW CODE----*/
   //Initialize the workspace (i.e., stack) and PD here!

   //Clear the contents of the workspace
   memset(&(p->workSpace),0,WORKSPACE);

   //Notice that we are placing the address (16-bit) of the functions
   //onto the stack in reverse byte order (least significant first, followed
   //by most significant).  This is because the "return" assembly instructions 
   //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig. 
   //second), even though the AT90 is LITTLE ENDIAN machine.

   //Store terminate at the bottom of stack to protect against stack underrun.
   *(unsigned char *)sp-- = ((unsigned int)Task_Terminate) & 0xff;
   *(unsigned char *)sp-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;
   *(unsigned char *)sp-- = 0x00;
   
   //Place return address of function at bottom of stack
   *(unsigned char *)sp-- = ((unsigned int)f) & 0xff;
   *(unsigned char *)sp-- = (((unsigned int)f) >> 8) & 0xff;
   *(unsigned char *)sp-- = 0x00;

#ifdef DEBUG
   //Fill stack with initial values for development debugging
   //Registers 0 -> 31 and the status register
   for (counter = 0; counter < 34; counter++)
   {
      *(unsigned char *)sp-- = counter;
   }
#else
   //Place stack pointer at top of stack
   sp = sp - 33;
#endif
      
   p->sp = sp;		/* stack pointer into the "workSpace" */
   p->code = f;		/* function to be executed as a task */
   p->request = NONE;
  p->id = (PID) Tasks;
  p->argument = arg;
  p->priority = py;


   p->state = READY;

}


/**
  *  Create a new task
  */
static void Kernel_Create_Task(void (*f)(void), PRIORITY py, int arg) 
{
   int x;

   if (Tasks == MAXPROCESS) return;  /* Too many task! */

   /* find a DEAD PD that we can use  */
   for (x = 0; x < MAXPROCESS; x++) {
       if (Process[x].state == DEAD) break;
   }

   ++Tasks;
   Kernel_Create_Task_At( &(Process[x]), f, py, arg);

}


/**
  * This internal kernel function is a part of the "scheduler". It chooses the 
  * next task to run, i.e., Cp.
  *TO DO: check for sus flags
  */
static void Dispatch()
{
     /* find the next READY task
       * Note: if there is no READY task, then this will loop forever!.
       */
   while(Process[NextP].state != READY) {
      NextP = (NextP + 1) % MAXPROCESS;
   }

   Cp = &(Process[NextP]);
   CurrentSp = Cp->sp;
   Cp->state = RUNNING;

   NextP = (NextP + 1) % MAXPROCESS;
}

/**
  * This internal kernel function is the "main" driving loop of this full-served
  * model architecture. Basically, on OS_Start(), the kernel repeatedly
  * requests the next user task's next system call and then invokes the
  * corresponding kernel function on its behalf.
  *
  * This is the main loop of our kernel, called by OS_Start().
  */
static void Next_Kernel_Request() 
{
   Dispatch();  /* select a new task to run */

   while(1) {
       Cp->request = NONE; /* clear its request */

       /* activate this newly selected task */
       CurrentSp = Cp->sp;
       Exit_Kernel();    /* or CSwitch() */

       /* if this task makes a system call, it will return to here! */

        /* save the Cp's stack pointer */
       Cp->sp = (unsigned char *) CurrentSp;

       switch(Cp->request){
        case CREATE:
           Kernel_Create_Task(Cp->code, Cp->priority, Cp->argument);
           break;
        case SUSPEND:
          Cp->sus = 1;
          Cp->state = READY;
          Dispatch();
          break;
       case NEXT:
	     case NONE:
           /* NONE could be caused by a timer interrupt */
          Cp->state = READY;
          Dispatch();
          break;
        case TERMINATE:
          /* deallocate all resources used by this task */
          Cp->state = DEAD;
          Dispatch();
          break;
        default:
          /* Houston! we have a problem here! */
          break;
       }
    } 
}


/*================
  * RTOS  API  and Stubs
  *================
  */

void OS_Abort(void);

/*================
  * TASK FUCNTIONS
  *================
  */

/**
  * Create task with given priority and argument, return the process ID
  */
PID Task_Create( void (*f)(void), PRIORITY py, int arg)
{
  if (KernelActive ) {
    Disable_Interrupt();
    Cp ->request = CREATE;
    Cp->code = f;
    Cp->id = (PID) Tasks;
    Cp->argument = arg;
    Cp->priority = py;
    Enter_Kernel();
  } else { 
    Kernel_Create_Task(f, py, arg);
  }

  return Cp->id;
}

/**
  * The calling task terminates itself.
  */
void Task_Terminate(void) {
   if (KernelActive) {
      Disable_Interrupt();
      Cp -> request = TERMINATE;
      Enter_Kernel();
     /* never returns here! */
   }
}

void Task_Yield(void){
  if (KernelActive) {
    Disable_Interrupt();
    Cp ->request = NEXT;
    Enter_Kernel();
    Enable_Interrupt();
  }
};

int Task_GetArg(void){
  return Cp->argument;
};

/**
* Suspends a task until it's handle is passed to resume
* Does nothing if the task is already suspended
*/
void Task_Suspend( PID p ){

  /* if the task to be suspended is currently running, then let the kernel get a new task running */
  if(Cp->id == p && KernelActive) {
      Disable_Interrupt();
      Cp->request = SUSPEND;
      Enter_Kernel();
      Enable_Interrupt();
  } else {
    int x;
    for(x=0; x < MAXPROCESS; x++) {
      
      if(Process[x].id == p) {
        Process[x].sus = 1;
      }

    }
  }
    

};

void Task_Resume( PID p ){

};

void Task_Sleep(TICK t){

};

/*================
  * MUTEX FUNCTIONS
  *================
  */

MUTEX Mutex_Init(void){

};

void Mutex_Lock(MUTEX m){

};

void Mutex_Unlock(MUTEX m){

};

/*================
  * EVENT FUNCTIONS
  *================
  */

EVENT Event_Init(void){

};

void Event_Wait(EVENT e){

};

void Event_Signal(EVENT e){

};


/**
  * This function initializes the RTOS and must be called before any other
  * system calls.
  */
void OS_Init() 
{
   int x;

   Tasks = 0;
   KernelActive = 0;
   NextP = 0;
	//Reminder: Clear the memory for the task on creation.
   for (x = 0; x < MAXPROCESS; x++) {
      memset(&(Process[x]),0,sizeof(PD));
      Process[x].state = DEAD;
   }
}

/**
  * This function starts the RTOS after creating a few tasks.
  */
void OS_Start(){
  if((! KernelActive) && (Tasks > 0)) {
    Disable_Interrupt();
    KernelActive = 1;
    Next_Kernel_Request();
  }
}

/**
  * The calling task gives up its share of the processor voluntarily.
  */
void Task_Next(){
  if (KernelActive) {
    Disable_Interrupt();
    Cp->request = NEXT;
    Enter_Kernel();
    Enable_Interrupt();
  }
}

// On interrupt switch task
ISR(TIMER1_COMPA_vect){
  if(KernelActive){
    Task_Yield();
    PORTB = 0x80;
  }
}

void Task2(){
  for(;;){
    PORTB = 0x40;
  }
}

void Task1()
{
  for(;;){
    PORTB = 0x20;
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
