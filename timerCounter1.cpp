//module for configuring and controlling the ATMega's timer counter 1
//used for the oven timer, the user can set a time for the oven to to cook. After time has expired,
//the oven stops cooking

#include "timerCounter1.h"
#include "oven_timer.h"
#include "monitor_log.h"
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef DEBUG_MODE
  #define LOG_TC1_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(TIMER_COUNTER1, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_TC1_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif


#ifdef EVENT_MODE
    #define LOG_TC1_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(TIMER_COUNTER1, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TC1_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_TC1_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(TIMER_COUNTER1, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TC1_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif

static void log_TC1_event(uint8_t type_ID, uint8_t event_ID, uint32_t time_stamp, uint16_t value)
{
  log_event(TIMER_COUNTER1, type_ID, event_ID, time_stamp, value);
}
void configure_timerCounter1(void)
{
  //timer counter 1, 1 tick per 1024 ticks of the system clock rate with max prescalar
  unsigned long prescaled_clock_rate = F_CPU / 1024;
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;

  TCNT1 = 0;
  //set timer to CTC mode, reset after interrupt
  TCCR1B |= (1 << WGM12);

  //compare value = approx 1 second
  OCR1A = prescaled_clock_rate;
  LOG_TC1_EVENT(TIMER_COUNTER1_CONFIGURED, millis(), 0);
}



// interrupt once per second, decrement until timer reaches 0 and cooking is finished 
ISR(TIMER1_COMPA_vect){
  if (get_timer_value() == 0)
  {
    //disable the timer interrupts and stop timer
    TIMSK1 = 0;
    TCCR1B &= ~((1 << CS12) | (1 << CS10));
    
  }
  else {decrement_timer();}

}

//disable the timer interrupts and stop timer
//used in oven_timer.ccp for when the user wants to stop or reset the timer before it has expired
void disable_timerCounter1(void)
{
  LOG_TC1_DEBUG(TIMER_COUNTER1_DISABLE, millis(), 0);
  TIMSK1 = 0;
  TCCR1B &= ~((1 << CS12) | (1 << CS10));
}
//for when the user begins the timer.
void enable_timerCounter1(void)
{
  LOG_TC1_DEBUG(TIMER_COUNTER1_ENABLE, millis(), 0);
  //enable compare A interrupts
  TIMSK1 |= (1 << OCIE1A);
  //start counting from 0
  TCNT1 = 0;
  //set prescalar to 1024 and start timer
  TCCR1B |= ((1 << CS12) | (1 << CS10));
}
