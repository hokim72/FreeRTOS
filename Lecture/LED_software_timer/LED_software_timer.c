#include <avr/io.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define LED_SPEED_MAX	5000
#define LED_SPEED_MIN	 100
#define LED_SPEED_STEP	  10

xTimerHandle xTimer1, xTimer2;

volatile portTickType ledSpeedControl;

void vTimerCallbackLED(xTimerHandle pxTimer)
{
	long lArrayIndex;

	lArrayIndex = (long) pvTimerGetTimerID(pxTimer);

	if (lArrayIndex == 1) PORTF ^= 0x03;
	if (lArrayIndex == 2) PORTF ^= 0xc0;
}

void vTask_speed_control()
{
	for (;;)
	{
		if ((PINK & 0x01) == 0x00)
		{
			ledSpeedControl += LED_SPEED_STEP;
			if (ledSpeedControl > LED_SPEED_MAX) ledSpeedControl = LED_SPEED_MAX;
			xTimerChangePeriod(xTimer1, ledSpeedControl, 10);
		}
		if ((PINK & 0x02) == 0x00)
		{
			ledSpeedControl -= LED_SPEED_STEP;
			if (ledSpeedControl < LED_SPEED_MIN) ledSpeedControl = LED_SPEED_MIN;
			xTimerChangePeriod(xTimer1, ledSpeedControl, 10);
		}

		vTaskDelay(100);
	}
}

int main(void)
{
	DDRF = 0xFF;
	PORTF = 0x03;

	DDRK = 0x00;
	PORTK = 0x03;

	ledSpeedControl = 500;

	// Create then start some timers. Starting the timers before the scheduler
	// has been started means the timers will start running immediately that
	// the scheduler starts. Care must be taken to ensure the timer command
	// queue is not filled up because the timer service task will not
	// start draining the timer command queue until after the scheduler has
	// been started.
	xTimer1 = xTimerCreate("Timer1", // Just a text name, not used by the kernel.
		500,						 // The timer period in ticks.
		pdTRUE,						 // The timers will auto-reload themselves when expire.
		(void*)(long) 1,			 // Assign each timer a unique id equal.
		vTimerCallbackLED			 // Each timer calls the same callback when it expires.
	);

	if (xTimer1 == NULL)
	{
		PORTF = 0x80;
	}
	else
	{
		// Start the timer. No block time is specified, even if one was it would be
		// ignored because the scheduler has not yet been started.
		if (xTimerStart(xTimer1, 0) != pdPASS)
		{
			PORTF = 0x40;
		}
	}

	xTimer2 = xTimerCreate("Timer2", 500, pdTRUE, (void*)(long)2, vTimerCallbackLED);
	xTimerStart(xTimer2, 0);

	xTaskCreate(
		vTask_speed_control,
		"vTask_speed_control",
		configMINIMAL_STACK_SIZE,
		NULL,
		tskIDLE_PRIORITY + 1, NULL
	);

	vTaskStartScheduler();

	for (;;);

	return 0;
}
