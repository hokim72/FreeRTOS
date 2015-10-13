#include <stdio.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainSENDER_1	1
#define mainSENDER_2	2

// Define the structure type that will be passed on the queue.
typedef struct
{
	unsigned char ucValue;
	unsigned char ucSource;
} xData;

// Declare two variables of type xData that will be passed on the queue.
static const xData xStructsToSend[2] =
{
	{ 100, mainSENDER_1 }, // Used by Sender1.
	{ 200, mainSENDER_2 }  // Used by Sender2.
};

xQueueHandle xQueue;

static void vSenderTask(void* pvParameters)
{
	BaseType_t xStatus;
	const portTickType xTicksToWait = 100 / portTICK_RATE_MS;

	// As per most tasks, this task is implemented within an infinite loop.
	for (;;)
	{
		// Send to the queue.

		// The second parameter is the address of the structure being sent.
		// The address is passed in as the task parameter so pvParameters is
		// used directly.

		// The third parameter is the Block time - the time the task should be
		// kept in the Blocked state to wait for space to become available on
		// the queue if the queue is already full. A block time is specified
		// because the sending tasks have a higher priority than the receiving
		// task so the queue is expected to become full. The receiving task
		// will remove items from the queue when both sending tasks are in
		// the Blocked state.
		xStatus = xQueueSendToBack(xQueue, pvParameters, xTicksToWait);

		if (xStatus != pdPASS)
		{
			// The send operation could not complete, even after waiting for
			// 100ms. This must be an error as the receiving task should make
			// space in the queue as soon as both sending tasks are in 
			// the Blocked state.
			vTaskSuspendAll();
			printf("Could not send to the queue.\n");
			fflush(stdout);
			xTaskResumeAll();

		}

		// Allow the other sender task to execute.
		taskYIELD();
	}
}

static void vReceiverTask(void* pvParameters)
{
	// Declare the structure that will hold the values received from the queue.
	xData xReceivedStructure;
	BaseType_t xStatus;

	// This task is also defined within an infinite loop.
	for (;;)
	{
		// Because it has the lowest priority this task will only run when the
		// sending tasks are in the Blocked state. The sending tasks will only
		// enter the Blocked state when the queue is full so this task always
		// expects the number of items in the queue to be equal to the queue
		// length - 3 in this case.
		if (uxQueueMessagesWaiting(xQueue) != 3)
		{
			vTaskSuspendAll();
			printf("Queue should have been full!\n");
			fflush(stdout);
			xTaskResumeAll();
		}

		// Receive from the queue.

		// The second parameter is the buffer into which the received data
		// will be placed. In this case the buffer is simply the address of
		// a variable that has the required size to hold the received structure.

		// The last parameter is the block time - the maximum amount of time
		// that the task will remain in the Blocked state to wait for data to
		// be available if the queue is already empty. In this case a block
		// time is not necessary because this task will only run when the
		// queue is full.
		xStatus = xQueueReceive(xQueue, &xReceivedStructure, 0);

		if (xStatus == pdPASS)
		{
			// Data was successfully received from the queue, print out the
			// received value and the source of the value.
			if (xReceivedStructure.ucSource == mainSENDER_1)
			{
				vTaskSuspendAll();
				printf("From Sender 1 = %d\n", xReceivedStructure.ucValue);
				fflush(stdout);
				xTaskResumeAll();
			}
			else
			{
				vTaskSuspendAll();
				printf("From Sender 2 = %d\n", xReceivedStructure.ucValue);
				fflush(stdout);
				xTaskResumeAll();
			}
		}
		else
		{
			// Nothing was received from the queue. This must be an error
			// as this task should only run when the queue is full.
			vTaskSuspendAll();
			printf("Could not receive from the queue.\n");
			fflush(stdout);
			xTaskResumeAll();
		}
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	// The queue is created to hold a maximum of 3 structures of type xData.
	xQueue = xQueueCreate(3, sizeof(xData));

	if (xQueue != NULL)
	{
		// Create two instances of the task that will write to the queue. The
		// parameter is used to pass the structure that the task will write to
		// the queue, so one task will continuously send xStructsToSend[ 0 ] to
		// the queue while the other task will continuously send 
		// xStructsToSend[ 1 ]. Both tasks are created at priority 2 which is
		// above the priority of the receiver.
		xTaskCreate(vSenderTask, "Sender1", configMINIMAL_STACK_SIZE*2,
		(void*)&xStructsToSend[0], tskIDLE_PRIORITY+2, NULL);
		xTaskCreate(vSenderTask, "Sender2", configMINIMAL_STACK_SIZE*2,
		(void*)&xStructsToSend[1], tskIDLE_PRIORITY+2, NULL);

		// Create the task that will read from the queue. The task is created
		// with priority 1, so below the priority of the sender tasks.
		xTaskCreate(vReceiverTask, "Receiver", configMINIMAL_STACK_SIZE*2,
					NULL, tskIDLE_PRIORITY+1, NULL);

		// Start the scheduler so the created tasks start executing.
		vTaskStartScheduler();
	}
	else
	{
		// The queue could not be created.
	}

	// If all is well then main() will never reach here as the scheduler will
	// now be running the tasks. If main() does reach here then it is likely
	// that there was insufficient heap memory available for the idle task to
	// be created. Chapter 5 provides more information on memory management.
	for (;;);

	return 0;
}
