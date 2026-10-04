#ifndef PIN_CHANGE_H
#define PIN_CHANGE_H
#include <stdint.h>

void new_attachPCINT(uint8_t PCMSK, uint8_t PCINT, uint8_t BIT, void (*ISR_function)(void), uint8_t mode);
void new_detachPCINT(uint8_t PCMSK, uint8_t PCINT);


#endif value