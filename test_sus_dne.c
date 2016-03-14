/* Tests if system crashes when a non existant process is suspended or resumed */

#include "os.h"
#include <avr/io.h>
#include <avr/interrupt.h>

unsigned int does_not_exist = 5;

void Task_1() {
	Task_Suspend(does_not_exist);

	Task_Resume(does_not_exist);
}


int a_main() {
	Task_Create(Task_1, 0, 0);

	Task_Terminate();
}