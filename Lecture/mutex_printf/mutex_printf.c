#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uartio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define SW_DEBOUNCE_TIME 20

xSemaphoreHandle xMutex;

xSemaphoreHandle xKeyInSemaphore = NULL;

xTaskHandle xHandleControl, xHandlePrint1, xHandlePrint2, xHandlePrint3;

void prvNewPrintString(const char* pcString)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	{
		printf("%s", pcString);
		fflush(stdout);
	}
	xSemaphoreGive(xMutex);
}

void prvPrintTask(void* pvParameters)
{
	char* pcStringToPrint;

	pcStringToPrint = (char*)pvParameters;

	for (;;)
	{
		prvNewPrintString(pcStringToPrint);
		vTaskDelay(rand() & 0x03FF);
	}
}

void vtaskControlTask(void* pvParameters)
{
	for (;;)
	{
		if (xSemaphoreTake(xKeyInSemaphore, portMAX_DELAY) == pdTRUE)
		{
			vTaskDelay(SW_DEBOUNCE_TIME);

			if ((PIND & 0x01) == 0x00)
			{
				vTaskSuspend(xHandlePrint1);
				PORTF &= ~(1 << 0);
			}

			if ((PIND & 0x02) == 0x00)
			{
				vTaskResume(xHandlePrint1);
				PORTF |= (1 << 0);
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

	uartIOInit();
	stdout = &uartIOOutput;

	EICRA = 0x0A;
	EIMSK = 0x03;
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
			tskIDLE_PRIORITY + 2,
			&xHandleControl
		);

		xTaskCreate(
			prvPrintTask,
			"vPrint1Task",
			configMINIMAL_STACK_SIZE*2,
			"1 Task 1 **********\r\n",
			tskIDLE_PRIORITY + 1,
			&xHandlePrint1
		);

		xTaskCreate(
			prvPrintTask,
			"vPrint2Task",
			configMINIMAL_STACK_SIZE*2,
			"2 Task 2 **********\r\n",
			tskIDLE_PRIORITY + 1,
			&xHandlePrint2
		);

		xTaskCreate(
			prvPrintTask,
			"vPrint3Task",
			configMINIMAL_STACK_SIZE*2,
			"3 Task 3 **********\r\n",
			tskIDLE_PRIORITY + 2,
			&xHandlePrint3
		);
	}
	PORTF = 0x7;

	vTaskStartScheduler();

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
