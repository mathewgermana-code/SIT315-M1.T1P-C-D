#include "ADC.h"
#include "monitor_log.h"
#include <avr/io.h>
#ifdef DEBUG_MODE
  #define LOG_ADC_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(ADC_, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_ADC_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_ADC_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(ADC_, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_ADC_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif

#ifdef ERROR_MODE
    #define LOG_ADC_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(ADC_ EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_ADC_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif






//the ADC is used by the potentiometer that acts as the user temperature select input.
//It is the only component that uses the ADC, so this configuration can remain the same for the
//program's duration. 

//- a note on power saving: both tinkercad and wokwi simulators don't implement the analogue
//comparator hardware on the ATMega328. If they did, the program would have been configured to put
//oven into a sleep or idle state to save power, and could be woken up if the input temperature selection
//had changed via the analogue compare interrupt, and by periodic timed interrupts to check the
//temp sensor reading, since the temperature changes are going to be slow and don't need to be checked
//every loop. Unfortunatley without the analogue comparator, to keep the program responsive the
//input temp selection has to be checked every loop and the oven can't be put into a sleep state. 
void configure_ADC(void)
{
  //ADC setup
  
  //select ADC pin mapping to A4 and reference voltage to (5V)
  //this means 5V is converted to 1023, 4V is converted to 4/5 * 1023 etc...
  ADMUX = 0;
  ADMUX |= (1 << MUX2) | (1 << REFS0);


  //setting prescalar and enabling ADC
  //The prescalar gives a scaled down clock frequency for the clock signal that drives the conversion
  //I know very little electronics theory, but the gist is the ADC has a series of operations,
  //these operations are driven by the clock signal, a faster clock means it will perform and complete
  //these operations faster, regardless of the validity of the result. A prescaled clockrate is
  //therefore essential for accurate results. 
  ADCSRA = 0;
  ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADCSRA |= (1 << ADEN);

  //disabling digital input buffer at DIAL_PIN
  //the digital input buffer is unused since we're doing analogue input, and is said to introduce 
  //electrical contamination for an analogue input, so it's best practice to disable it. 
  DIDR0 |= (1 << ADC4D);

  LOG_ADC_EVENT(ADC_CONFIGURED, millis(), 0);
  //trigger initial conversion
  //Writing a 1 to the ADSC bit begins conversion, essentially the first part of the arduino 
  //analogueRead()
  ADCSRA |= (1 << ADSC);
}

//Writing a 1 to the ADSC bit in this register begins conversion, after conversion it is automatically
//set back to 0.
bool ADC_conversion_state(void)
{
  //check if ADC is currently converting
  return (ADCSRA & (1 << ADSC)) == true;
}

void ADC_start_conversion(void)
{
  LOG_ADC_DEBUG(ADC_START_CONVERSION, millis(), 0);
  ADCSRA |= (1 << ADSC);
}