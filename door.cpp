#include "door.h"
#include "oven_timer.h"
#include "monitor_log.h"
#include <avr/io.h>

#ifdef DEBUG_MODE
  #define LOG_DOOR_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(DOOR, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_DOOR_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_DOOR_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(DOOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DOOR_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_DOOR_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(DOOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_DOOR_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif


//module for the light sensor which is used as the door open/closed sensor. In real life an optical
//based sensor would not likely be used for this, rather some sort of phyisical switch moved by
//door opening and closing, unfortunatley neither Wokwi nor Tinkercad have such a component.
//That said some door/lid devices do use an optical sensor for this kind of thing. like some
//video game consoles and dvd and blu-ray players for the disc compartment, and some printers. 



//PinChangeInterrupt.h handles a the repetative tasks in configuring pin change interrupts.
//i.e. writing the appropriate register configuration to YYY to enable that pin to cause interrupts
//implementing the ISR function logic that determines which function to run depending on what pins
//changed. The pins in this category are part of a port, one interrupt exists for any change in the
//whole port (it may be more apt to call it a port change interrupt). So to allow for different ISRs
//for individual pins, the port change ISR uses pointers to other functions and decides which to execute
//based on the old and new pin states.

#include <PinChangeInterrupt.h>

//light sensor used for determining if the door is open or closed
static const int PR_PIN = 2;
//current door state
static volatile bool door_open;

//--initialized in configure_door()--
//the bit position of this pin in its various registers
  static uint8_t bit_pos;
  //label for the port that the PR_PIN belongs to
  static uint8_t port_label;
  //The PIN register bit for a pin (input) gives its current state.
  //address of the PIN register for the given port label
  static volatile uint8_t *port_reg;


static void door_ISR()
{
  uint8_t door_open_local;
  //check the current value on the PIN register bit corresponding to the door sensor
  door_open_local = (*port_reg & bit_pos);
  if(door_open_local) {LOG_DOOR_EVENT(DOOR_OPENED, millis(), 0);}
  else {LOG_DOOR_EVENT(DOOR_CLOSED, millis(), 0);}
  //If the timer has expired, the user can stop and reset it by simply opening the door rather than having
  //to push the start reset button explicitly. 
  if (is_timer_expired())
  {
    signal_start_reset();
    LOG_DOOR_EVENT(DOOR_TM_RESET, millis(), 0);
  }
  door_open = door_open_local;
}

void configure_door(void)
{
  pinMode(PR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PR_PIN), door_ISR, CHANGE);
  door_open = digitalRead(PR_PIN);

  //the bit position of this pin in its various registers
  bit_pos = digitalPinToBitMask(PR_PIN);
  //label for the port that the PR_PIN belongs to
  port_label = digitalPinToPort(PR_PIN);
  //The PIN register bit for a pin (input) gives its current state.
  //address of the PIN register for the given port label
  port_reg = portInputRegister(port_label); 

  LOG_DOOR_EVENT(DOOR_CONFIGURED, millis(), 0);
}

bool is_door_open(void)
{
  //contradiction between door_open value and pin state
  if (!((*port_reg & bit_pos) ^ !door_open))
  {LOG_DOOR_ERROR(DOOR_STATE_ERROR, millis(), door_open);}

  return door_open;
}


