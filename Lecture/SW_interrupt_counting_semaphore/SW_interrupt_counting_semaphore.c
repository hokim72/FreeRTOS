#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

xSemaphoreHandle xCountingSemaphore = NULL;
static char sw_counter;

static void vSW_counterHandlerTask(void* pvParameters)
{
	sw_counter = 0x00;

	for (;;)
	{
		if (xSemaphoreTake(xCountingSemaphore, portMAX_DELAY) == pdTRUE)
		{
			if ((PIND & 0x01) == 0)
			{
				sw_counter++;
			}
		}
	}
}

static void vLED_displayTask(void* pvParameters)
{
	PORTF = 0xff;
	vTaskDelay(500);
	PORTF = 0x00;
	vTaskDelay(500);

	for (;;)
	{
		PORTF = sw_counter;
		vTaskDelay(20);
	}
}

int main(void)
{
	DDRD = 0x00;
	PORTD = 0xFF;
	DDRF = 0xFF;
	PORTF = 0x00;

	EICRA = 0x02;
	EIMSK |= 0x01;

	sei();

	xCountingSemaphore = xSemaphoreCreateCounting(10, 0);

	if (xCountingSemaphore != NULL) {
		xTaskCreate(
			vSW_counterHandlerTask,
			"vSW_counterHandlerTask",
			configMINIMAL_STACK_SIZE*3,
			NULL,
			tskIDLE_PRIORITY + 3,
			NULL
		);

		xTaskCreate(
			vLED_displayTask,
			"vLED_displayTask",
			configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 1, 
			NULL
		);

		vTaskStartScheduler();
	}

	for (;;) {}

	return 0;
}

ISR(INT0_vect)
{
	static BaseType_t xHigherPriorityTaskWoken;
	BaseType_t xSemaStatus;

	EIMSK &= ~0x01;

	xHigherPriorityTaskWoken = pdFALSE;

	xSemaStatus = xSemaphoreGiveFromISR(xCountingSemaphore, &xHigherPriorityTaskWoken);

	EIMSK |= 0x01;

	if ((xSemaStatus == pdTRUE) && (xHigherPriorityTaskWoken == pdTRUE))
	{
		taskYIELD();
	}
}
