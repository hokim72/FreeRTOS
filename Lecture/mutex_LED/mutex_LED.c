#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define SW_DEBOUNCE_TIME	20

xSemaphoreHandle xMutex;

xSemaphoreHandle xKeyInSemaphore = NULL;

xTaskHandle xHandleControl, xHandleLED1, xHandleLED2, xHandleLED3;

void vLedControl(char ledNo)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	{
		PORTF &= ~0x0F;
		PORTF |= (1 << ledNo);

		vTaskDelay(100);
	}

	xSemaphoreGive(xMutex);
}

void vLEDTask(void* pvParameters)
{
	char LEDTaskNo;

	LEDTaskNo = *(char *)pvParameters;
	LEDTaskNo -= '0';

	for (;;)
	{
		vLedControl(LEDTaskNo);

		vTaskDelay((rand() & 0x3FF));
	}
}

void vtaskControlTask(void* pvParameters)
{
	PORTF = 0x80;

	for (;;)
	{
		if (xSemaphoreTake(xKeyInSemaphore, portMAX_DELAY) == pdTRUE)
		{
			vTaskDelay(SW_DEBOUNCE_TIME);
			if ((PIND & 0x01) == 0x00) {
				vTaskSuspend(xHandleLED1);
				PORTF = 0x40;
			}
			if ((PIND & 0x02) == 0x00) {
				vTaskResume(xHandleLED1);
				PORTF = 0x80;
			}
			EIMSK |= 0x03;
		}
	}
}

int main(void)
{
	DDRD = 0x00;
	PORTD = 0xFF;
	DDRF = 0xFF;
	PORTF = 0x00;

	EICRA = 0x0A;
	EIMSK |= 0x03;

	sei();

	xMutex = xSemaphoreCreateMutex();
	vSemaphoreCreateBinary(xKeyInSemaphore);

	srand(567);

	if ((xMutex != NULL) & (xKeyInSemaphore != NULL))
	{
		xTaskCreate(
			vtaskControlTask,
			"vtaskControlTask",
			configMINIMAL_STACK_SIZE,
			NULL,
			tskIDLE_PRIORITY + 4, 
			&xHandleControl
		);

		xTaskCreate(
			vLEDTask,
			"vLED1Task",
			configMINIMAL_STACK_SIZE,
			"0",
			tskIDLE_PRIORITY + 1,
			&xHandleLED1
		);

		xTaskCreate(
			vLEDTask,
			"vLED2Task",
			configMINIMAL_STACK_SIZE,
			"1",
			tskIDLE_PRIORITY + 1,
			&xHandleLED2
		);

		xTaskCreate(
			vLEDTask,
			"vLED3Task",
			configMINIMAL_STACK_SIZE,
			"2",
			tskIDLE_PRIORITY + 2,
			&xHandleLED3
		);

		vTaskStartScheduler();
	}

	for (;;) {}
	return 0;
}

ISR(INT0_vect)
{
	static BaseType_t xHigherPriorityTaskWoken;

	EIMSK &= ~0x03;

	xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(xKeyInSemaphore, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken == pdTRUE)
	{
		taskYIELD();
	}
}

ISR(INT1_vect, ISR_ALIASOF(INT0_vect));


