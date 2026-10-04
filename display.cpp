#include "display.h"
#include "oven_timer.h"
#include "monitor_log.h"
#include <Arduino.h>
//function for formatting and outputting information for the serial monitor
#ifdef DEBUG_MODE
  #define LOG_DISPLAY_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(TM1637, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_DISPLAY_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif


#ifdef EVENT_MODE
    #define LOG_DISPLAY_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(TM1637, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DISPLAY_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_DISPLAY_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(TM1637, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DISPLAY_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif




//abstraction library for the TM1637 display
//TMI1637 used for displaying timer value
#include <DIYables_4Digit7Segment_TM1637.h>
//this part of the program relies on the above library
//TM1637 uses a two wire, sychronised, bi-directional protocol - the MCU sends commands and the display
//returns acknowledgements. 
//essentially, a byte corresponds to a digit, and each bit in the byte corresponds to which segment
//in that digit gets lit up. The display has a register with an address for each digit. The MCU
//issues a START signal to begin transmission, a configuration command ('data command'), an address command
//to indicate the digit being written to, and a bit sequence containing the segment display mapping.
//It can also send a display command to turn the display on or off, and adjust its brightness.
//That is a general overview of the operations the library handles for this program. 

//pins for the TMI1637 4 digit display
static const int clock_pin = 4;
static const int data_pin = 5;
DIYables_4Digit7Segment_TM1637 display(clock_pin, data_pin);

void configure_display(void)
{
  display.begin();
  //save power by making the display's default state off.
  display.off();
  LOG_DISPLAY_EVENT(DISPLAY_CONFIGURED, millis(), 0);
}

//called in the main loop to decide to update the display, switch it on or off. 
void update_display(void)
{ 
  static uint8_t old_timer_value = 0;
  uint8_t new_timer_value = get_timer_value();
  //only update the display if the timer value has changed
  if (new_timer_value != old_timer_value)
  {
    LOG_DISPLAY_DEBUG(DISPLAY_UPDATED, millis(), new_timer_value);
    if (new_timer_value == 0) {display.off();} //issue an off command if timer has changed to 0
    else if (old_timer_value == 0) {display.on();} //issue an on command if timer has changed from 0
    old_timer_value = new_timer_value;
    display.print(new_timer_value);
  }
}

