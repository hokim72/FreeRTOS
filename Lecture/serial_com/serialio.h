#ifndef SERIALIO_H
#define SERIALIO_H
#include <stdio.h>

void serialIOPutchar(char c, FILE *stream);
char serialIOGetchar(FILE *stream);
void serialIOInit(uint8_t len);

extern FILE serialIOOutput;
extern FILE serialIOInput;

#endif
