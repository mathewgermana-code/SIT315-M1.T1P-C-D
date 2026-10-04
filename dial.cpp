#include "dial.h"
#include "ADC.h"
#include "monitor_log.h"

//module for handling the potentiometer, the input dial for the user to set the desired oven temp.

#ifdef DEBUG_MODE
  #define LOG_DIAL_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(DIAL, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_DIAL_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_DIAL_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(DIAL, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DIAL_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif

#ifdef ERROR_MODE
    #define LOG_DIAL_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(DIAL, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DIAL_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif


//input pin for the potentiometer that sets the desired oven cooking temperature
static const int dial_pin = A4;

void configure_dial(void)
{
  configure_ADC();
  LOG_DIAL_EVENT(DIAL_CONFIGURED, millis(), 0);
}

//function for obtaining and interpereting the potentiometer input
//
unsigned int get_selected_temp(void)
{
  
  static unsigned int old_dial_result = 0; //used purley for serial monitor logging purpose
  uint16_t dial_result = ADCL; //set dial_result to the last ADC conversion
  dial_result |= ((uint16_t)ADCH << 8);

  

  if(ADC_conversion_state() == false) {


    LOG_DEBUG(ADC_, ADC_CONVERT_DONE, millis(), dial_result >> 2);

    ADC_start_conversion();
  
  } //begin new conversion if last requested conversion is complete

  if (dial_result > 1023) {
    LOG_ERROR(ADC_, ADC_VALUE_ERROR, millis(), dial_result >> 2);
    dial_result = 0;
  }


  else if (dial_result > 0) {
    //dial_result /= 5.688888; //scale the ADC range of 0-1024 to approx 0-180
  
    //floating point maths is inefficient!, integer multiplication and bit shift division (powers of 2) is better
    //division equals multiplying by recipricol, 5.68888 = 256/45
    dial_result = ((dial_result * 45) >> 8);
  
    dial_result += 149; // make the min temp above 0 be 150°C
  }
  

  if(dial_result != old_dial_result) 
  {
    LOG_DIAL_EVENT(DIAL_TEMP_CHANGE, millis(), dial_result>>2);

    old_dial_result = dial_result;
  }
  
  return dial_result;
  
}