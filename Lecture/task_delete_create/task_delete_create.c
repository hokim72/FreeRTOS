#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"

#define CREATED_TASK 1
#define DELETED_TASK 0

char cLEDTask1Flag = CREATED_TASK, cLEDTask2Flag = CREATED_TASK;

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

void vtaskControlTask(void* pvParameters)
{
	char cSWStatus;

	for (;;)
	{
		if ((PINK & 0x03) != 0x03) vTaskDelay(20);

		if ((PINK & 0x03) != 0x03)
		{
			cSWStatus = PINK & 0x03;

			if (cSWStatus == 0x02)
			{
				if (cLEDTask1Flag == CREATED_TASK)
				{
					vTaskDelete(xHandleLED1);
					cLEDTask1Flag = DELETED_TASK;
					PORTF = 0x00;
				}
			}

			if (cSWStatus == 0x01)
			{
				if (cLEDTask2Flag == CREATED_TASK)
				{
					vTaskDelete(xHandleLED2);
					cLEDTask2Flag = DELETED_TASK;
					PORTF = 0x00;
				}
			}
		}
		else
		{
			if (cLEDTask1Flag == DELETED_TASK)
			{
				xTaskCreate(
					vLEDTask1,
					"vLEDTask1",
					configMINIMAL_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + 2, 
					&xHandleLED1
				);
				cLEDTask1Flag = CREATED_TASK;
			}
			if (cLEDTask2Flag == DELETED_TASK)
			{
				xTaskCreate(
					vLEDTask2,
					"vLEDTask2",
					configMINIMAL_STACK_SIZE,
					NULL,
					tskIDLE_PRIORITY + 2,
					&xHandleLED2
				);
				cLEDTask2Flag = CREATED_TASK;
			}
		}
	}
}

void vApplicationIdleHook(void)
{

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
		(const char* const)"vLEDTask2",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 2,
		&xHandleLED2
	);

	vTaskStartScheduler();

	for (;;) {}

	return 0;
}

