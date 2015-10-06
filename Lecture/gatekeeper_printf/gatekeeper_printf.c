#include <stdio.h>
#include <stdlib.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

static char* pcStringsToPrint[] =
{
	"Task 1 ****************\r\n",
	"Task 2 ----------------\r\n",
	"Tick hook interrupt ###\r\n"
};

xQueueHandle xPrintQueue;

void prvPrintTask(void* pvParameters)
{
	int iIndexToString;

	iIndexToString = (int) pvParameters;

	for (;;)
	{
		xQueueSendToBack(xPrintQueue, &pcStringsToPrint[iIndexToString], 0);

		vTaskDelay(rand() & 0x03FF);
	}
}


void prvStdioGatekeeperTask(void* pvParameters)
{
	char* pcMessageToPrint;

	for (;;)
	{
		xQueueReceive(xPrintQueue, &pcMessageToPrint, portMAX_DELAY);

		printf("%s", pcMessageToPrint);
		fflush(stdout);
	}
}

void vApplicationTickHook(void)
{
	static int iCount = 0;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	iCount++;
	if (iCount >= 600)
	{
		xQueueSendToFrontFromISR(xPrintQueue, &pcStringsToPrint[2], &xHigherPriorityTaskWoken);

		iCount = 0;
	}
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	xPrintQueue = xQueueCreate(5, sizeof(char*));

	srand(567);

	if (xPrintQueue != NULL)
	{
		xTaskCreate(
			prvPrintTask,
			"vPrint1Task",
			configMINIMAL_STACK_SIZE*2,
			(void*)0,
			tskIDLE_PRIORITY+1,
			NULL
		);

		xTaskCreate(
			prvPrintTask,
			"vPrint2Task",
			configMINIMAL_STACK_SIZE*2,
			(void*)1,
			tskIDLE_PRIORITY+2,
			NULL
		);

		xTaskCreate(
			prvStdioGatekeeperTask,
			"Gatekeeper",
			configMINIMAL_STACK_SIZE*2,
			NULL,
			tskIDLE_PRIORITY,
			NULL
		);
	}

	vTaskStartScheduler();

	for (;;) {}

	return 0;

}

