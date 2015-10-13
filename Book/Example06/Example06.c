#include <stdio.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"

void vContinuousProcessingTask(void* pvParameters)
{
	char* pcTaskName;

	// The sting to print out is passed in via the parameter. Cast this to a
	// character pointer.
	pcTaskName = (char*)pvParameters;

	// As per most tasks, this task is implemented in an infinite loop.
	for (;;)
	{
		// Print out the name of this task. This task just does this repeatedly
		// without ever blocking or delaying.
		vTaskSuspendAll();
		printf(pcTaskName);
		fflush(stdout);
		xTaskResumeAll();
	}
}

void vPeriodicTask(void* pvParameters)
{
	portTickType xLastWakeTime;

	// The xLastWakeTime variable needs to be initialized with the current tick
	// count. Note that this is the only time the variable is written to
	// explicitly. After this xLastWakeTime is updated automatically
	// internally within vTaskDelayUntil().
	xLastWakeTime = xTaskGetTickCount();

	// As per most tasks, this task is implemented in an infinite loop.
	for (;;)
	{
		// Print out the name of this task.
		vTaskSuspendAll();
		printf("Periodic task is running......\n");
		fflush(stdout);
		xTaskResumeAll();

		// This task should execute exactly every 250 milliseconds exactly.
		vTaskDelayUntil(&xLastWakeTime, (100/portTICK_RATE_MS));
	}
}

// Define the strings that will be passed in as the task parameters. These are
// defined const and not on the stack to ensure they remain valid when the tasks
// are executing.
static const char* pcTextForTask1 = "Continuous task 1 is running\n";
static const char* pcTextForTask2 = "Continuous task 2 is running\n";

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	// Create the first task at priority 1. The priority is the second to last
	// parameter.
	xTaskCreate(vContinuousProcessingTask, "Task 1", configMINIMAL_STACK_SIZE*2, 
				 (void*)pcTextForTask1, tskIDLE_PRIORITY+1, NULL); 
	
	// Create the second task at priority 1. 
	xTaskCreate(vContinuousProcessingTask, "Task 2", configMINIMAL_STACK_SIZE*2,
				(void*)pcTextForTask2, tskIDLE_PRIORITY+1, NULL);

	xTaskCreate(vPeriodicTask, "Task 3", configMINIMAL_STACK_SIZE*2,
				NULL, tskIDLE_PRIORITY+2, NULL);

	// Start the scheduler so our tasks start executing
	vTaskStartScheduler();

	// If all is well then main() will never reach here as the scheduler will
	// now be running the tasks. If main() does reach here then it is likely
	// that there was insufficient heap memory available for the idle task 
	// to be created.
	// Chapter 5 provides more information on memory management.
	for (;;);

	return 0;
}
