//The module for controlling the heat element, for the simulation it is simply represented with an
//LED

//A note on the hardware configuration: in addition to being regulated by the programs internal
//logic with the door sensor, smoke sensor, temp sensor and timer, the heat element on signal comes through
//an AND gate. One of its inputs is from the arduino, but one is the digital output of the smoke
//sensor (which output LOW for a smoke detected reading). If the smoke level is high or the smoke
//sensor gets disconnected somehow, the heat element physically can't turn on.

#include "heat_element.h"
#include "temp.h"
#include "dial.h"
#include "smoke.h"
#include "door.h"
#include "oven_timer.h"
#include "monitor_log.h"

#ifdef DEBUG_MODE
  #define LOG_HEAT_ELEMENT_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(HEAT_ELEMENT, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_HEAT_ELEMENT_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_HEAT_ELEMENT_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(HEAT_ELEMENT, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_HEAT_ELEMENT_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_HEAT_ELEMENT_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(HEAT_ELEMENT, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_HEAT_ELEMENT_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif

//output pin for the LED that represents the heating element in the oven
static const int heat_element_pin = 13;

//the oven pin will be exposed to smoke.c for safety with get_HE_pin()
//heat_element_pin could be made non-static and be declared in heat_element.h. However this would
//give access to the heat element pin through main, or any other module might include the heat_element.h
//interface which is less safe. Only smoke.cpp will be told about the existence of this external function.
uint8_t get_HE_pin(void)
{
  return heat_element_pin;
}

static uint8_t HEP_bit_pos;
static uint8_t HEP_port_label;
static volatile uint8_t *HEP_out_reg;

void configure_heat_element()
{
  pinMode(heat_element_pin, OUTPUT);

    //heating element pin port & pin bit position:

  //bit position for the heat element pin
  HEP_bit_pos = digitalPinToBitMask(heat_element_pin); 
  //port label for the heat element pin
  HEP_port_label = digitalPinToPort(heat_element_pin);
  //address of the PORT register for the given port label
  HEP_out_reg = portOutputRegister(HEP_port_label);
  LOG_HEAT_ELEMENT_EVENT(HEAT_ELEMENT_CONFIGURED, millis(), 0);

  configure_dial();
  configure_temp_sensor();

}

//regularly turns the heat element on and off based on the detected and selected temperature
//To avoid rapid switching between on and off, it has a 5 degree hysteresis band.
static bool determine_heat_state(void)
{
  float oven_temp = get_oven_temp_celcius();
  float selected_temp = get_selected_temp();
  static bool oven_state = false;

  //activate heating element
  //only turns back on when oven temperature falls 2.5 degrees below selected temp.
  if (oven_temp < selected_temp-2.5)
  {
    if (oven_state == LOW) {LOG_EVENT(TEMP_SENSOR, TEMP_RECEEDED, millis(), oven_temp);}
    oven_state = HIGH;
  }
  //deactivate heating element
  //only turns it back off when oven temp exceeds selected temp by 2.5 degrees
  else if (oven_temp >= selected_temp+2.5)
  {
    if (oven_state == HIGH) {LOG_EVENT(TEMP_SENSOR, TEMP_EXCEEDED, millis(), oven_temp);}
    oven_state = LOW;
  }
  return oven_state;
}
//called in the main loop, decides whether write high or low to the heat element pin
void set_heat_state(void)
{
  static bool old_heat_state = false;


  //conditions for writing LOW to the heat element pin
  //if smoke has been detected (even if not current), the door is open, the timer is expired
  //or determine_heat_state determined LOW, write LOW.
  if(has_smoke_detected() || is_door_open() || is_timer_expired() || !determine_heat_state())
  {
    //writing 0 to the heat element's bit position in the relevant PIN register
    //equivilent to digitalWrite()
    *HEP_out_reg &= ~(HEP_bit_pos);
    if (old_heat_state == true)
    {
      old_heat_state = false;
      LOG_HEAT_ELEMENT_EVENT(HEAT_ELEMENT_OFF, millis(), 0);
    }
    return;
  }
  //write 1 to the heat element bit in the relevent PIN register
  else {
    *HEP_out_reg |= (HEP_bit_pos);
    if (old_heat_state == false)
    {
      old_heat_state = true;
      LOG_HEAT_ELEMENT_EVENT(HEAT_ELEMENT_ON, millis(), 0);
    }
  
  }
  

}