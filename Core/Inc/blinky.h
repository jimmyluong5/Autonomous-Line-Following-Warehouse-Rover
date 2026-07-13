#ifndef BLINKY_H
#define BLINKY_H

#include <stdbool.h>

void Blinky_Init(void);
bool Blinky_update(void);
#endif

// these are just defining function prototypes
// so that main.c can call them