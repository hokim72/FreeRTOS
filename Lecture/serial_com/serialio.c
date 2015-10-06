#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "serialio.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#ifndef F_CPU
#define F_CPU 16000000UL
#endif
#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>

static xQueueHandle xRxedChars;
static xQueueHandle xCharsForTx;

FILE serialIOOutput = FDEV_SETUP_STREAM(serialIOPutchar, NULL, _FDEV_SETUP_WRITE);
FILE serialIOInput = FDEV_SETUP_STREAM(NULL, serialIOGetchar, _FDEV_SETUP_READ);

void serialIOInit(uint8_t len) {

	xRxedChars = xQueueCreate(len, sizeof(char));
	xCharsForTx = xQueueCreate(len, sizeof(char));

	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0); /* Enable RX and TX */
}

void serialIOPutchar(char c, FILE *stream) {
	if (c == '\n') {
		serialIOPutchar('\r', stream);
	}
	if (xQueueSend(xCharsForTx, &c, 0) == pdPASS)
	{
		UCSR0B |= (1 << UDRIE0);
	}
}

char serialIOGetchar(FILE *stream) {
	char cChar;

	xQueueReceive(xRxedChars, &cChar, 0);

	return cChar;
}

ISR(USART0_RX_vect)
{
	char cChar;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	cChar = UDR0;

	xQueueSendFromISR(xRxedChars, &cChar, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken != pdFALSE)
	{
		taskYIELD();
	}
}

ISR(USART0_UDRE_vect)
{
	char cChar;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (xQueueReceiveFromISR(xCharsForTx, &cChar, &xHigherPriorityTaskWoken) == pdTRUE)
	{
		UDR0 = cChar;
	}
	else
	{
		UCSR0B &= ~(1 << UDRIE0);
	}
}
