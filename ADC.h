#ifndef ADC_H
#define ADC_H
#include <Arduino.h>
void configure_ADC(void);
bool ADC_conversion_state(void);
void ADC_start_conversion(void);

#endif