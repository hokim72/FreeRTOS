#include <stdio.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"

// Declare a variable that is used to hold the handle of Task 2.
xTaskHandle xTask2Handle;

void vTask1(void* pvParameters)
{
	UBaseType_t uxPriority;

	// This task will always run before Task2 as it is created with the higher
	// priority. Neither Task1 nor Task2 ever block so both will always be
	// in either the Running or the Ready state.
	
	// Query the priority at which this task is running - passing in NULL means
	// "return my priority".
	uxPriority = uxTaskPriorityGet(NULL);

	for (;;)
	{
		// Print out the name of this task.
		printf("Task 1 is running\n");
		fflush(stdout);

		// Setting the Task 2 priority above the Task 1 priority will cause
		// Task2 to immediately start running (as then Task 2 will have the 
		// higher priority of the two created tasks). Note the use of the handle
		// to task 2 (xTask2Handle) in the call to vTaskPrioritySet().
		// main() shows how the handle was obtained.
		printf("About to raise the Task 2 priority\n");
		fflush(stdout);
		vTaskPrioritySet(xTask2Handle, uxPriority+1);

		// Task 1 will only run when it has a priority higher than Task 2.
		// Therefore, for this task to reach this point Task2 must already
		// have executed and set its priority back down to below the priority
		// of this task.
	}
}

void vTask2(void* pvParameters)
{
	UBaseType_t uxPriority;

	// Task 1will always run before this task as Task 1 is created with the
	// higher priority. Neither Task 1 nor Task 2 ever block so will always be
	// in either the Running or the Ready state.
	
	// Query the priority at which this task is running - passing in NULL means
	// "return my priority".
	uxPriority = uxTaskPriorityGet(NULL);

	for (;;)
	{
		// For this task to reach this point Task 1 must have already run and
		// set the priority of this task higher than its own.
		
		// Print out the name of this task.
		printf("Task2 is running\n");
		fflush(stdout);

		// Set our priority back down to its original value. Passing in NULL
		// as the task handle means "change my priority". Setting the priority
		// below that of Task 1 will cause Task 1 to immediately start running
		// again - pre-empting this task.
		printf("About to lower the Task 2 priority\n");
		fflush(stdout);
		vTaskPrioritySet(NULL, uxPriority-2);
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	// Create the first task at priority 2. The task parameter is not used
	// and set th NULL. The task handle is also not used so is also set to NULL.
	xTaskCreate(vTask1, "Task 1", configMINIMAL_STACK_SIZE*2, NULL,
							tskIDLE_PRIORITY+2, NULL);
	// The task is create at priority 2 _____^.

	// Create the second task at priority 1 - which is lower than the priority
	// given to Task 1. Again the task parameter is not used so is set to NULL -
	// BUT this time the task handle is required so the address of xTask2Handle
	// is passed in the last parameter.
	xTaskCreate(vTask2, "Task 2", configMINIMAL_STACK_SIZE*2, NULL,
							tskIDLE_PRIORITY+1, &xTask2Handle);
	// The task handle is the last parameter ___^^^^^^^^^^^^^.

	// Start the scheduler so the tasks start executing.
	vTaskStartScheduler();

	// If all is well then main() will never reach here as the scheduler will
	// now be running the tasks. If main() does reach here then it is likely
	// that there was insufficient heap memory available for the idle task to
	// be created. Chapter 5 provides more information on memory management.
	for (;;);

	return 0;
}

