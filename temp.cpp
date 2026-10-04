//module for configuring and reading data from the temp sensor. 
//This temp sensor uses the one wire protocol. A bit too complex for me to describe in detail
//Essentially the protocol defines a time based encoding, the MCU master holding the pin LOW for some
//amount of time is taken as a 0, or a 1 for some other amount of time, or a command like read, write or restart. The sensor
//like wise outputs data with the same encoding for the MCU to read from.
//DallasTemperature is the higher level protocol that describe specific values and what commands they equate to.
//It has clever functionality for communicating with multiple sensor modules, although we don't use that here.

//note that the decision for this sensor was purley because it has the widest temperature range of the temp sensors included in 
//the simulator, it was NOT to avoid using the ADC for more than one component, this was just a convenient coinsidence.

#include "temp.h"
#include "monitor_log.h"
//Libraries for communicating with the temp sensor
#include <OneWire.h>
#include <DallasTemperature.h>

#ifdef DEBUG_MODE
  #define LOG_TEMP_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(TEMP_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_TEMP_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif


#ifdef EVENT_MODE
    #define LOG_TEMP_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(TEMP_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TEMP_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_TEMP_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(TEMP_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TEMP_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif


//input pin for the temperature sensor, used for regulating the oven temperature
static const int temp_sensor_pin = A0;

static OneWire oneWire(temp_sensor_pin);
static DallasTemperature sensors(&oneWire);
static DeviceAddress temp_sensor_address;

void configure_temp_sensor(void)
{
  sensors.begin(); // for temp sensor
  //get the temp sensor address
  sensors.getAddress(temp_sensor_address, 0); 

  //don't halt execution while waiting for conversion
  sensors.setWaitForConversion(false);
  //do an initial request
  sensors.requestTemperatures();
  LOG_TEMP_EVENT(TEMP_SENSOR_CONFIGURED, millis(), 0);

}

//requests temp conversion, returns last temp output if 1000ms hasn't passed or the conversion isn't complete, or both
//(the latter is unlikley as conversion takes about 750ms)
float get_oven_temp_celcius(void)
{ 
  //defined once, updated at the first function call after 1000ms
  static unsigned long last_get_temp = 0;
  static uint8_t old_temp = 0;

  //return value, defined once
  static int16_t tempC = 150;
  if (millis() - last_get_temp > 1000)
  { 
    //assign most recent temp reading to tempC
    tempC = sensors.getTempC(temp_sensor_address);
    LOG_TEMP_DEBUG(TEMP_SENSOR_CONVERT_DONE, millis(), tempC>>1);
    //request new conversion
    sensors.requestTemperatures();
    //time stamp for this function call
    last_get_temp = millis();
    //note: starting the temperatures at 150 degress is just for the purposes of demonstration in the simulator
    //no temp sensors are provided that reach the true internal temperature of the oven
    tempC = tempC + 55 + 150;
  }
  if((tempC < 0) || (tempC > 330))
  {
    LOG_TEMP_ERROR(TEMP_SENSOR_VALUE_ERROR, millis(), tempC >> 1);
    return UINT8_MAX;
  }
  if(old_temp != (tempC >> 1))
  {
    old_temp = (tempC>>1);
    LOG_TEMP_EVENT(TEMP_SENSOR_CHANGE, millis(), old_temp);
  }
  return tempC;

}