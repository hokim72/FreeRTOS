#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

void vTask1_flash_led(void* pvParameters)
{
	portTickType xLastWakeTime;

	xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		PORTF = ~PORTF;
		vTaskDelayUntil(&xLastWakeTime, 500);
	}
}

void vApplicationIdleHook(void)
{

}

int main(void)
{
	DDRF = 0xFF;
	PORTF = 0x00;

	xTaskCreate(
		vTask1_flash_led,
		"vTask1_flash_led",
		configMINIMAL_STACK_SIZE, 
		NULL, 
		tskIDLE_PRIORITY + 1,
		NULL
	);

	vTaskStartScheduler();

	for (;;);

	return 0;
}
