#include <stdio.h>
#include <util/delay.h>

#include "uartio.h"
//#include "serialio.h"

#include "FreeRTOS.h"
#include "task.h"

void vTask1(void* pvParameters)
{
	const char* pcTaskName = "Task 1 is running\n";

	// As per most tasks, this task is implemented in an infinite loop.
	for (;;)
	{
		// Print out the name of this task.
		printf(pcTaskName);
		fflush(stdout);

		// Delay for a period
		// This loop is just a very crude delay implementation. There is
		// nothing to do in here. Later examples will replace this crude
		// loop with a proper delay/sleep function.
		_delay_ms(200);
	}
}

void vTask2(void* pvParameters)
{
	const char* pcTaskName = "Task 2 is running\n";

	// As per most tasks, this task is implemented in an infinite loop.
	for (;;)
	{
		// Print out the name of this task.
		printf(pcTaskName);
		fflush(stdout);

		// Delay for a period
		// This loop is just a very crude delay implementation. There is
		// nothing to do in here. Later examples will replace this crude
		// loop with a proper delay/sleep function.
		_delay_ms(200);
		//for (uint32_t i=0; i<500000; i++);
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;
	//serialIOInit(64);
	//stdout = &serialIOOutput;

	// Create one of the two tasks. Note that a real application should check
	// the return value of the xTaskCreate() call to ensure the task was created
	// successfully.
	xTaskCreate( vTask1, // Pointer to the function that implements the task.
				 "Task 1", // Text name for the task. This is to facilitate debugging only.
				 configMINIMAL_STACK_SIZE*2, // Stack depth in words.
				 NULL, // We are not using the task parameter.
				 tskIDLE_PRIORITY+1, // This task will run at priority 1.
				 NULL ); // We are not going to use the task handle.

	// Create the other task in exactly the same way and at the same priority.
	xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY+1, NULL);

	// Start the scheduler so the tasks start executing.
	vTaskStartScheduler();

	// If all is well then main() will never reach here as the scheduler will
	// now be running the tasks. If main() does reach here then it is likely that
	// there was insufficient heap memory available for the idle task to be created.
	// Chapter 5 provides more information on memory management.
	for (;;);

	return 0;
}
