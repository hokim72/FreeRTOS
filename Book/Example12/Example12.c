#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

xSemaphoreHandle xBinarySemaphore;

static void vPeriodicTask(void* pvParameters)
{
	// As per most tasks, this task is implemented within an infinite loop.
	for (;;)
	{
		// This task is just used to 'simulate' an interrupt. This is done by
		// periodically generating a software interrupt.
		vTaskDelay(500/portTICK_RATE_MS);

		// Generate the interrupt, printing a message both before hand and
		// afterwards so the sequence of execution is evident from the output.
		printf("Periodic task - About to generate an interrupt.\n");
		fflush(stdout);
		PORTD &= ~(1 << PD0);
		PORTD |= (1 << PD0);
		printf("Periodic task - Interrupt generated.\n\n");
		fflush(stdout);
	}
}

static void vHandlerTask(void* pvParameters)
{
	// As per most tasks, this task is implemented within an infinite loop.

	// Task the semaphore once to start with so the semaphore is empty before
	// the infinite loop is entered. The semaphore was created before the 
	// scheduler was started so before this task ran for the first time.
	xSemaphoreTake(xBinarySemaphore, 0);

	for (;;)
	{
		// Use the semaphore to wait for the event. The task blocks
		// indefinitely meaning thin function call will only return once the
		// semaphore has been successfully obtained - so there is no need to
		// check the returned value.
		xSemaphoreTake(xBinarySemaphore, portMAX_DELAY);

		// To get here event must have occured. Process the event (in this
		// case we just print out a message).
		printf("Handler task - Processing event.\n");
		fflush(stdout);
	}
}

ISR(INT0_vect)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// 'Give' the semaphore to unblock the task.
	xSemaphoreGiveFromISR(xBinarySemaphore, &xHigherPriorityTaskWoken);

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

	// Before a semaphore is used it must be explicitly created. In this example
	// a binary semaphore is created.
	vSemaphoreCreateBinary(xBinarySemaphore);

	// Check the semaphore was created successfully.
	if (xBinarySemaphore != NULL)
	{
		// Enable the software interrupt and set its priority.
		EIMSK |= (1 << INT0);
		EICRA |= (1 << ISC00) | (1 << ISC01);
		sei();

		// Create the 'handler' task. This is the task that will be synchronized
		// with the interrupt. The handler task is created with a  high priority
		// to ensure it runs immediately after interrupt exits. In this case a
		// priority of 3 is chosen.
		xTaskCreate(vHandlerTask, "Handler", configMINIMAL_STACK_SIZE*2, NULL,
					tskIDLE_PRIORITY+3, NULL);

		// Create the task that will periodically generate a software interrupt.
		// This is created with a priority below the handler task to ensure it 
		// will get preempted each time the handler task exits the Blocked
		// state.
		xTaskCreate(vPeriodicTask, "Periodic", configMINIMAL_STACK_SIZE*2, NULL,
					tskIDLE_PRIORITY+1, NULL);

		// Start the scheduler so the created tasks start executing.
		vTaskStartScheduler();

	}

	// If all is well we will never reach here as the scheduler will now be
	// running the tasks. If we do reach here then it is likely that there was
	// insufficient heap memory available for a resource to be created.
	for (;;);

	return 0;
}
