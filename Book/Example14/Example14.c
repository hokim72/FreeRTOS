#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Declare two variable of type QueueHandle. One queue will be read from
// within an ISR, the other will be written to from within an ISR.
xQueueHandle xIntegerQueue, xStringQueue;

static void vIntegerGenerator(void* pvParameters)
{
	portTickType xLastExecutionTime;
	unsigned long ulValueToSend = 0;
	int i;

	// Initialize the variable used by the call to vTaskDelayUntil().
	xLastExecutionTime = xTaskGetTickCount();
	for (;;)
	{
		// This is a periodic task. Block until it is time to run again.
		// The task will execute every 200ms.
		vTaskDelayUntil(&xLastExecutionTime, 200/portTICK_RATE_MS);

		// Send an incrementing number to the queue five times. The values
		// will be read from the queue by the interrupt service routine.
		// The interrupt service routine always empties the queue so this
		// task is guaranteed to be able to write all five values, so
		// a block time is not required.
		for (i=0; i<5; i++)
		{
			xQueueSendToBack(xIntegerQueue, &ulValueToSend, 0);
			ulValueToSend++;
		}

		// Force an interrupt so the interrupt service routine can read the
		// values from the queue.
		vTaskSuspendAll();
		printf("Generator task - About to generate an interrupt.\n");
		fflush(stdout);
		xTaskResumeAll();
		PORTD &= ~(1 << PD0);
		PORTD |= (1 << PD0);
		vTaskSuspendAll();
		printf("Generator task - Interrupt generated.\n\n");
		fflush(stdout);
		xTaskResumeAll();
	}
}

static void vStringPrinter(void* pvParameters)
{
	char* pcString;

	for (;;)
	{
		// Block on the queue to wait for data to arrive.
		xQueueReceive(xStringQueue, &pcString, portMAX_DELAY);

		// Print out the string received.
		vTaskSuspendAll();
		printf(pcString);
		fflush(stdout);
		xTaskResumeAll();
	}
}

ISR(INT0_vect)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static unsigned long ulReceivedNumber;

	// The strings are declared static const to ensure they are not allocated
	// to the interrupt service routine stack, and exist even when the
	// interrupt service routine is not executing.
	static const char* pcStrings[] =
	{
		"String 0\n",
		"String 1\n",
		"String 2\n",
		"String 3\n"
	};

	// Loop until the queue is empty.
	while (xQueueReceiveFromISR(xIntegerQueue, &ulReceivedNumber,
								&xHigherPriorityTaskWoken) != errQUEUE_EMPTY)
	{
		// Truncate the received value to the last two bits (value 0 to 3 inc.),
		// then send the string that corresponds to the truncated value to the
		// other queue.
		ulReceivedNumber &= 0x03;
		xQueueSendToBackFromISR(xStringQueue,
								&pcStrings[ulReceivedNumber],
								&xHigherPriorityTaskWoken);
	}

	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		// Giving the semaphore may have unblocked a task - if it did and the
		// unblocked task has a priority equal to or above the currently
		// executing task then xHigherPriorityTaskWoken will have been set to
		// pdTRUE and vPortYield() will force a context switch to the newly
		// unblocked higher priority task.

		// NOTE: The syntax for forcing a context switch within an ISR varies 
		// between FreeRTOS ports.
		vPortYield();
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	DDRD |= (1 << PD0);
	PORTD &= ~(1 << PD0);

	// Before a queue can be used it must first be created. Create both queues
	// used by this example. One queue can hold varaiables of type unsigned
	// long, the other queue can hold variables of type char*. Both queues
	// can hold a maximum of 10 items. A real application should check the
	// return values to ensure the queues have successfully created.
	xIntegerQueue = xQueueCreate(10, sizeof(unsigned long));
	xStringQueue = xQueueCreate(10, sizeof(char*));

	// Enable the software interrupt and set its priority.
	EIMSK |= (1 << INT0);
	EICRA |= (1 << ISC00) | (1 << ISC01);
	sei();

	// Create the task that uses a queue to pass integers to the interrupt
	// service routine. The task is created at priority 1.
	xTaskCreate(vIntegerGenerator, "IntGen", configMINIMAL_STACK_SIZE*2,
				NULL, tskIDLE_PRIORITY+1, NULL);

	// Create the task the prints out the strings sent to it from the interrupt
	// service routine. This task is created at the higher priority of 2.
	xTaskCreate(vStringPrinter, "String", configMINIMAL_STACK_SIZE*2,
				NULL, tskIDLE_PRIORITY+2, NULL);

	// Start the scheduler so the created tasks start executing.
	vTaskStartScheduler();

	// If all is well we will never reach here as the scheduler will now be
	// running the tasks. If we do reach here then it is likely that there was
	// insufficient heap memory available for a resource to be created.
	for (;;);

	return 0;
}
