#include <stdio.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t ulIdleCycleCount = 0UL;

void vIdleCycleCountPrintTask(void* pvParameters)
{
	printf("IdleCycleCount Test\r\n");
	vTaskDelay(100);

	for (;;)
	{
		printf("IdleCount: %lu \r\n", ulIdleCycleCount);
		vTaskDelay(500);
	}
}

void vApplicationIdleHook(void)
{
	ulIdleCycleCount++;
}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	xTaskCreate(
		vIdleCycleCountPrintTask,
		"IdleCycleCountPrint",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 1,
		NULL
	);

	vTaskStartScheduler();

	for (;;) {}

	return 0;
}
