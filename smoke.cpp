//code for configuring and controlling the smoke sensor
#include "smoke.h"
#include "monitor_log.h"
#include <PinChangeInterrupt.h>
#include <avr/io.h>

#ifdef DEBUG_MODE
  #define LOG_SMOKE_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(SMOKE_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_SMOKE_DEBUG(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef EVENT_MODE
    #define LOG_SMOKE_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(SMOKE_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_SMOKE_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_SMOKE_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(SMOKE_SENSOR, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_SMOKE_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif

//----------------------------------------------------------------------------------------------------------------

//input pin for the MQ2 gas/smoke sensor, used to trigger a safety mechanism
static const uint8_t smoke_sensor_pin = 8;
//the function defined in heat_element.cpp, exclusivley for smoke.cpp to obtain the private value.
extern uint8_t get_HE_pin(void);
static uint8_t heat_element_pin; // initialised in the finish_MQ2_setup()


//has smoke been detected at any point?
//If excess smoke is detected, the user has to manually hit reset or turn off the oven for it to
//start cooking again. 
static volatile bool smoke_detected;


//is smoke currently detected?
static volatile bool current_smoke_state;

static void smoke_ISR(void)
{
  //This is one of the few ISRs that performs an actual operation - turning off the heat element -
  //rather than merely signaling the main thread of execution to do so.
  //Since this is a safetfy feature it seems appropriate. 
  //heating element PORT register & pin bit position:
  static uint8_t HEP_bit_pos = digitalPinToBitMask(heat_element_pin);
  static uint8_t HEP_port_label = digitalPinToPort(heat_element_pin);
  static volatile uint8_t *HEP_port_out_reg = portOutputRegister(HEP_port_label);
  //smoke sensor PIN register and pin bit position
  static uint8_t MQ2_bit_pos = digitalPinToBitMask(smoke_sensor_pin);
  static uint8_t MQ2_port_label = digitalPinToPort(smoke_sensor_pin);
  static volatile uint8_t *MQ2_port_input_reg = portInputRegister(MQ2_port_label);
  //direct digital read
  current_smoke_state = !(*MQ2_port_input_reg & MQ2_bit_pos);
  if (current_smoke_state)
  {
    //disable heating element, write LOW
    *HEP_port_out_reg &= (~(1 << HEP_bit_pos));
    smoke_detected = true;
    LOG_SMOKE_EVENT(MQ2_HIGH, millis(), 0);
    LOG_SMOKE_EVENT(HEAT_ELEMENT_OFF, millis(), 0);
  }
  else{LOG_SMOKE_EVENT(MQ2_LOW, millis(), 0);}
  
}

/*
The following is an arrangement to make the main program ignore the MQ2 sensor for the first 10 seconds of the program,
the reasons for which is described in the README. 
Both getter functions get_current_smoke_state and has_smoke_detected have a 'pre' and 'post' and _ptr co-function.
In the first 10 seconds of the program, calls two main getter functions return calls to the 'pre' versions. After 10
seconds the finish_MQ2_setup() function assigns the 'post' versions to the _ptr pointers, from then on the calls to
the post versions are returned*/

//----------------------get_current_smoke_state----------------------------------------------
bool (*get_current_smoke_state_ptr)(void); //functions pointer for the getter of current_smoke_state

//the version of the getter that will be called in the first 10 seconds
bool get_current_smoke_state_pre(void)
{
  return false;
}
//the version called after 10 seconds
bool get_current_smoke_state_post(void)
{
  return current_smoke_state;
}


//the actual function presented in smoke.h called by other code modules
//returns a call to the function pointed to by get_current_smoke_state_ptr
bool get_current_smoke_state(void)
{
  return (*get_current_smoke_state_ptr)();
}

//----------------------has_smoke_detected---------------------------------------
//the version of the getter that will be called in the first 10 seconds
bool (*has_smoke_detected_ptr)(void);

//the version of the getter that will be called in the first 10 seconds
bool has_smoke_detected_pre(void)
{
  return false;
}

//the version called after 10 seconds
bool has_smoke_detected_post(void)
{
  return smoke_detected;
}

//the actual function presented in smoke.h called by other code modules, returns a call to the function pointed to by
//has_smoke_detected_ptr
bool has_smoke_detected(void)
{
  return (*has_smoke_detected_ptr)();
}
//----------------------------------------------------------

//the initial setup called at the start of the program, both the getter functions will return false causing the program to 
//disregard their result

void configure_smoke_sensor(void)
{
  pinMode(smoke_sensor_pin, INPUT);
  heat_element_pin = get_HE_pin();
  get_current_smoke_state_ptr = &get_current_smoke_state_pre;
  has_smoke_detected_ptr = &has_smoke_detected_pre;

}

//the second setup called after 10 seconds have passed, re-assigned the getter function pointers to the propper definitions
//that return the real values
void finish_MQ2_setup()
{
  get_current_smoke_state_ptr = &get_current_smoke_state_post;
  has_smoke_detected_ptr = &has_smoke_detected_post;
  attachPCINT(digitalPinToPCINT(smoke_sensor_pin), smoke_ISR, CHANGE);
  smoke_ISR();
  LOG_SMOKE_EVENT(MQ2_CONFIGURED, millis(), 0);

}




