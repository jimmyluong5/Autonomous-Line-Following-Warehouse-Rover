#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

void Encoder_Init(void);
int16_t Encoder_GetRightCount(void);
int16_t Encoder_GetLeftCount(void);
void Encoder_ResetRight(void);
void Encoder_ResetLeft(void);
void Encoder_Update(void);

#endif
