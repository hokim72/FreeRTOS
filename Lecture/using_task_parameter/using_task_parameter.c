#include <stdio.h>
#include <util/delay.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"

static const char* pcTextForTask1 = " Task 1 is Running \r\n";
static const char* pcTextForTask2 = " Task 2 is Running \r\n";

void vTaskFunction(void* pvParameters)
{

	for (;;)
	{
		printf((char*)pvParameters);
		fflush(stdout);

		_delay_ms(20);
	}
}

void vApplicationIdleHook(void)
{

}

int main(void)
{
	uartIOInit();
	stdout = &uartIOOutput;

	xTaskCreate(
		vTaskFunction,
		"Task1",
		configMINIMAL_STACK_SIZE*2,
		(void*)pcTextForTask1,
		tskIDLE_PRIORITY + 1,
		NULL
	);

	xTaskCreate(
		vTaskFunction,
		"Task2",
		configMINIMAL_STACK_SIZE*2,
		(void*)pcTextForTask2,
		tskIDLE_PRIORITY + 1,
		NULL
	);



	vTaskStartScheduler();

	for (;;);

	return 0;
}
