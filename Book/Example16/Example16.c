#include <stdio.h>
#include <stdlib.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Define the strings  that the tasks and interrupt will print out via the
// gatekeeper.
static char* pcStringsToPrint[] =
{
	"Task 1 ************************************************\n",
	"Task 2 ************************************************\n",
	"Message printed from the tick hook interrupt ##########\n"
};

// Declare a variable of type xQueueHandle. This is used to send messages from
// the print tasks and the tick interrupt to the gatekeeper task.
xQueueHandle xPrintQueue;

static void prvStdioGatekeeperTask(void* pvParameters)
{
	char* pcMessageToPrint;

	// This is the only task that is allowed to write to the terminal output.
	// Any other task wanting to write a string to the output does not access
	// the terminal directly, but instead sends the string to this task. As
	// only this task accesses standard out there are no mutual exclusion or 
	// serialization issues to consider within the implementation of the
	// task itself.
	for (;;)
	{
		// Wait for a message to arrive. An indefinite block time is specified
		// so there is no need to check the return value - the function will 
		// only return when a message has been successfully received.
		xQueueReceive(xPrintQueue, &pcMessageToPrint, portMAX_DELAY);

		// Output the received string
		printf(pcMessageToPrint);
		fflush(stdout);

		// Now go back to wait for the next message.
	}
}

static void prvPrintTask(void* pvParameters)
{
	int iIndexToString;

	// Two instances of this task are created. The task parameter is used to
	// pass an index into an array of strings into the task. Cast this to
	// the required type.
	iIndexToString = (int)pvParameters;

	for (;;)
	{
		// Print out the string, not directly but instead by passing a pointer
		// to the string to the gatekeeper task via a queue. The queue is
		// created before the scheduler is started so will already exist by
		// the time this task executes for the first time. A block time is
		// not specified because there should always be space in the queue.
		xQueueSendToBack(xPrintQueue, &pcStringsToPrint[iIndexToString], 0);

		// Wait a pseudo random time. Note that rand() is not necessarily
		// reentrant, but in this case it doest not really matter as the code
		// does not care what value is returned. In a more secure application
		// a version of rand() that is known to be reentrant should be used
		// - or calls to rand() should be protected using a critical section.
		vTaskDelay(rand() & 0x1FF);
	}
}

void vApplicationTickHook(void)
{
	static int iCount = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Print out a message every 200 ticks. The message is not written out
	// directly, but sent to the gatgekeeper task.
	iCount++;
	if (iCount >= 200)
	{
		// In this case the last parameter (xHigherPriorityTaskWoken) is not
		// actually used but must still be supplied.
		xQueueSendToFrontFromISR(xPrintQueue,
								&pcStringsToPrint[2],
								&xHigherPriorityTaskWoken);

		// Reset the count ready to print out the string again in 200 ticks
		// time.
		iCount = 0;
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	// Before a queue is used it must be explicitly created. The queue is
	// created to hold a maximum of 5 character pointers.
	xPrintQueue = xQueueCreate(5, sizeof(char*));

	// The tasks are going to use a pseudo random delay, seed the random number
	// generator.
	srand(567);

	// Check the queue was created successfully.
	if (xPrintQueue != NULL)
	{
		// Create two instances of the tasks that send messages to the 
		// gatekeeper. The index to the string the task uses is passed to
		// the task via the task parameter (the 4th parameter to xTaskCreate()).
		// The tasks are created at different priorities so the higher
		// priority task will occasionally preempt the lower priority task.
		xTaskCreate(prvPrintTask, "Print1", configMINIMAL_STACK_SIZE*2,
			(void*)0, tskIDLE_PRIORITY+1, NULL);
		xTaskCreate(prvPrintTask, "Print2", configMINIMAL_STACK_SIZE*2,
			(void*)1, tskIDLE_PRIORITY+2, NULL);

		// Create the gatekeeper task. This is the only task that is permitted
		// to directly access standard out.
		xTaskCreate(prvStdioGatekeeperTask, "Gatekeeper", configMINIMAL_STACK_SIZE*2,
			NULL, tskIDLE_PRIORITY, NULL);

		// Start the scheduler so the created tasks start executing.
		vTaskStartScheduler();

	}
	// If all is well we will never reach here as the scheduler will now be
	// running the tasks. If we do reach here then it is likely that there was
	// insufficient heap memory available for a resource to be created.
	for (;;);

	return 0;
}
