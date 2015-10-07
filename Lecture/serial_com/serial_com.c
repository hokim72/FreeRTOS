#include <stdio.h>

#include "serialio.h"

#include "FreeRTOS.h"
#include "task.h"

void vComRxTxTestTask(void* pvParameters)
{
	int inChar;

	printf("Serial Comm. Test\n");
	fflush(stdout);
	vTaskDelay(100);
	for (;;)
	{
		inChar = getchar();
		putchar((char)inChar);
	}
}

int main(void)
{
	serialIOInit(64);
	stdout = &serialIOOutput;
	stdin = &serialIOInput;

	xTaskCreate(
		vComRxTxTestTask,
		"COMRxTx",
		configMINIMAL_STACK_SIZE*2,
		NULL,
		tskIDLE_PRIORITY + 2,
		NULL
	);

	vTaskStartScheduler();

	for (;;) {}

	return 0;
}
