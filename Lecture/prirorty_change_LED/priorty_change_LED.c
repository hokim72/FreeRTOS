#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

xTaskHandle xHandleControl, xHandleLED1, xHandleLED2;

void vLEDTask1(void* pvParameters)
{
	for (;;)
	{
		PORTF ^= 0x0F;
		vTaskDelay(500);
	}
}

void vLEDTask2(void* pvParameters)
{
	for (;;)
	{
		PORTF ^= 0xF0;
		vTaskDelay(500);
	}
}

void vtaskControlTask(void*pvParameters)
{
	for (;;)
	{
		if ((PINK & 0x03) == 0x03) {
			vTaskPrioritySet(xHandleLED1, tskIDLE_PRIORITY + 2);
			vTaskPrioritySet(xHandleLED2, tskIDLE_PRIORITY + 2);
		}
		if ((PINK & 0x01) == 0x00) {
			vTaskPrioritySet(xHandleLED1, tskIDLE_PRIORITY + 2);
			vTaskPrioritySet(xHandleLED2, tskIDLE_PRIORITY + 1);
		}
		if ((PINK & 0x02) == 0x00) {
			vTaskPrioritySet(xHandleLED1, tskIDLE_PRIORITY + 1);
			vTaskPrioritySet(xHandleLED2, tskIDLE_PRIORITY + 2);
		}
	}
}

int main(void)
{
	DDRK = 0x00;
	PORTK = 0xFF;

	DDRF = 0xFF;
	PORTF = 0x00;

	xTaskCreate(
		vtaskControlTask, 
		"vtaskControlTask", 
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 2,
		&xHandleControl
	);

	xTaskCreate(
		vLEDTask1,
		"vLEDTask1",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 2,
		&xHandleLED1
	);

	xTaskCreate(
		vLEDTask2,
		"vLEDTask2",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 2,
		&xHandleLED2
	);

	vTaskStartScheduler();

	for (;;) {}

	return 0;
}
