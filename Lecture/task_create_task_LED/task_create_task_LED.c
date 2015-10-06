#include <avr/io.h>
#include <stdio.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"

void vTask1_flash_led(void* pvParameters) 
{
	xTaskCreate(
			vTask2_flash_led, 
			"vTask2_flash_led", 
			configMINIMAL_STACK_SIZE, 
			NULL, 
			tskIDLE_PRIORITY + 1, 
			NULL);
	
	for (;;)
	{
		PORTF ^= 0x0F;
		vTaskDelay(500);	// 500mSec Delay
	}
}

void vTask2_flash_led(void* pvParameters) 
{

	for (;;)
	{
		vTaskDelay(500);	// 500mSec Delay
		PORTF ^= 0xF0;
	}
}

void vApplicationIdleHook(void)
{

}

int main(void) 
{

	uartIOInit();
	stdout = &uartIOOutput;

	DDRF = 0xFF; // PORTF : LED Port
	PORTF = 0x00; // Clear led

	// Create Tasks
	xTaskCreate(
			vTask1_flash_led, 
			"vTask1_flash_led", 
			configMINIMAL_STACK_SIZE, 
			NULL, 
			tskIDLE_PRIORITY + 1, 
			NULL);

	// Run Schedular
	vTaskStartScheduler();

	for (;;);

	return 0;
}
