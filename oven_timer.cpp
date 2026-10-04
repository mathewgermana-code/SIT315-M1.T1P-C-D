//code for the software level oven timer
//timer counter 1 only generates an interrupt every second, oven_timer abstracts this functionality
//into a high level user-facing timer by tracking and modifying the timer value (cooking time remaining).
//it provides abstractions for other modules that need to obtain or modify the timer contents i.e. 
//the display module display.cpp to output the value to the display, or buzzer.cpp which generates
//a buzzer tone for when the timer has expired.
#include "oven_timer.h"
#include "timerCounter1.h"
#include "monitor_log.h"
#ifdef DEBUG_MODE
  #define LOG_OVEN_TIMER_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(OVEN_TIMER, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_OVEN_TIMER_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_OVEN_TIMER_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(OVEN_TIMER, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_OVEN_TIMER_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_OVEN_TIMER_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(OVEN_TIMER, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_OVEN_TIMER_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif

static void log_oven_timer_event(uint8_t type_ID, uint8_t event_ID, uint32_t time_stamp, uint16_t value)
{
  log_event(HEAT_ELEMENT, type_ID, event_ID, time_stamp, value);
}




//timer value (in seconds for demonstration, can be changed to minutes)
//8 bit int so the MCU can perform atomic 1 cycle reads & writes
//an interrupt occuring inbetween read & write cycles can create issues
static volatile uint8_t timer_value = 0;
// timer running, == true, timer not running == false
static volatile bool timer_state = false;

static const uint8_t timer_max = 180; //max value for the timer. The type is 8 bits, but the display only has room for 4 digits, could go up to 9999.

//used for interrupts to signal that the user has requested the timer to start or stop/reset.
//designed for the dedicated start/reset button which can signal at any time, but opening the oven 
//door or pressing any of the other buttons will reset the timer if it has gone from active to expired
static bool start_reset_pending = false;

void configure_oven_timer(void)
{
  configure_timerCounter1();
  LOG_OVEN_TIMER_EVENT(OVEN_TIMER_CONFIGURED, millis(), 0);
}

//functions for obtaining the timer state and vlaueare used by door.cpp, timer_buttons.cpp 
//timer_couter1.cpp and display.cpp, mainly for deciding to signal a start/reset. buzzer.cpp uses it
// to decide on generating a time alert tone. 
uint8_t get_timer_value(void)
{
  if (timer_value > timer_max) {LOG_OVEN_TIMER_ERROR(TIMER_VALUE_ERROR, millis(), timer_value);}
  return timer_value;
}

bool get_timer_state(void)
{
  if(((!timer_state) && ((TIMSK1 & OCIE1A) || (TCCR1B & (CS12 | CS11 | CS10)))) ||
  (timer_state && (!(TIMSK1 & OCIE1A) || !(TCCR1B & (CS12 | CS10)))) )
  {LOG_OVEN_TIMER_ERROR(TIMER_STATE_CONFIG_ERROR, millis(), timer_state);}
  return timer_state;
}
bool is_timer_expired(void)
{
  //return true when the timer was started and has now reached 0
  return (timer_state && timer_value == 0);
}
//-------------------------------------------------------------------------------------
//interface for modifying the timer value, which is done by timer_buttons and timer_counter1.
void set_timer_zero(void)
{
  timer_value = 0;
}

void set_timer_max(void)
{
  timer_value = timer_max;
}

uint8_t increment_timer(void)
{
  //increment timer_value up to timer_max, then roll back to 0
  if (timer_value++ == timer_max) {timer_value = 0;}
  return timer_value;
}
uint8_t decrement_timer(void)
{
  //decrement timer_value down to 0, then roll back to timer_max
  if (timer_value-- == 0) {timer_value = timer_max;}
  return timer_value;
}


void flip_timer_state(void)
{
  timer_state = !timer_state;
}

//------------------------------------------------------------------------
//interface for the start/stop and reset functionality. 
//A function can signal a request for a start or reset, i.e. the start/reset button handler function
void signal_start_reset(void)
{
  LOG_OVEN_TIMER_DEBUG(SIGNAL_START_RESET, millis(), timer_state);
  start_reset_pending = true;
}
//start_reset_handler is called repeatadley in the main loop. If a start/reset is pending, it checks
//it checks the current timer_state and value to decide what to do
void start_reset_handler(void)
{
  if (start_reset_pending)
  {
  
    start_reset_pending = false;
  //timer is already going, stop and reset.
    if (timer_state)
    {
      //disable the timer counter 1 interrupts and stop timer
      disable_timerCounter1();
   
      LOG_OVEN_TIMER_EVENT(OVEN_TIMER_STOPPED, millis(), timer_value);
      timer_value = 0;
      timer_state = false;
    }
    //timer is stopped, begin timer
    //if timer is stopped but timer_value is 0, do nothing.
    else if (timer_value > 0)
    {
      LOG_OVEN_TIMER_EVENT(OVEN_TIMER_STARTED, millis(), timer_value);
      enable_timerCounter1();
      timer_state = true;
    }
  }
}