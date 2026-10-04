//This is the code module for the timer buttons. 3 in total: timer up, timer down and start/reset.
//For the demonstration the unit of the timer is seconds, this would be changed to minutes in a real device.
//In the stopped state, timer_state == false: This is when the user selects a timer value before starting the timer
//timer up will increment timer_value, timer down will decrement timer_value, which is on the display.
//The start/reset button will put the timer in the active state if timer_value is not 0.

//In the active state, timer_state == true: timer counter 1 will be started and timer_value will be decremented
//once per second until reaching 0. timer up and down will still change the timer_value, in case the user wants to increase or
//decrease the time remaining, but the countdown will continue like normal until reaching 0 (again, this feature makes more
//sense with minutes as the unit rather than seconds). When timer_value reaches 0, the timer is still in the active state and
//the buzzer will sound until the user causes a start reset signal, which can be done by pressing any of the three buttons or
//opening the oven door. If start/reset button is pushed before timer_value reaches 0, the timer will go back to the stopped 
//state, and timer_value will go back to 0. 



#include <Arduino.h>
#include "timer_buttons.h"
#include "oven_timer.h"
#include "monitor_log.h"

#ifdef DEBUG_MODE
  #define LOG_TM_BUTTONS_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(TIMER_BUTTONS, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_TM_BUTTONS_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif


#ifdef EVENT_MODE
    #define LOG_TM_BUTTONS_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(TIMER_BUTTONS, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TM_BUTTONS_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif


#ifdef ERROR_MODE
    #define LOG_TM_BUTTONS_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(TIMER_BUTTONS, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_TM_BUTTONS_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif
static void log_tm_buttons_event(uint8_t type_ID, uint8_t event_ID, uint32_t time_stamp, uint16_t value)
{
  log_event(TIMER_BUTTONS, type_ID, event_ID, time_stamp, value);
}

#include <PinChangeInterrupt.h>
//#include <PinChangeInterruptBoards.h>
//#include <PinChangeInterruptPins.h>
//#include <PinChangeInterruptSettings.h>

//button for increasing the timer value
static const int timer_up_button_pin = 3;
#define timer_up_button_BIT PD3
#define timer_up_button_DDR DDRD
#define timer_up_button_PORT PORTD
#define timer_up_button_PIN PIND
#define timer_up_button_PCINT PCINT19
#define timer_up_button_PCMSK PCMSK2
//button for decreasing the timer value
static const int timer_down_button_pin = 6;
#define timer_down_button_BIT PD6
#define timer_down_button_DDR DDRD
#define timer_down_button_PORT PORTD
#define timer_down_button_PIN PIND
#define timer_down_button_PCINT PCINT22
#define timer_down_button_PCMSK PCMSK2
//Button for starting and reseting the timer
static const int timer_SR_button = 9;
#define timer_SR_button_BIT PB1
#define timer_SR_button_DDR DDRB
#define timer_SR_button_PORT PORTB
#define timer_SR_button_PIN PINB
#define timer_SR_button_PCINT PCINT1
#define timer_SR_button_PCMSK PCMSK0

static const int bounce_buffer = 400;

//these are the variables that the button ISRs use to signal to the main thread. handler functions are called in the main loop
//which acts on these variables rather than the ISRs themselves doing bulky operations
static volatile bool SR_button_pending = false;
static volatile bool timer_up_button_pending = false;
static volatile bool timer_down_button_pending = false;


static int up_button_PCINT = digitalPinToPCINT(timer_up_button_pin);
static int down_button_PCINT = digitalPinToPCINT(timer_down_button_pin);

static int SR_button_PCINT = digitalPinToPCINT(timer_SR_button);

void new_pinMode(uint8_t BIT, uint8_t PORT, uint8_t DDR, uint8_t mode)
{
  //input
  if (mode == 0) {DDR &= ~(1 << BIT);}
  //output
  else if (mode == 1) {DDR |= (1 << BIT);}
  //input pullup
  else if(mode == 2) {
    DDR &= ~(1 << BIT);
    PORT |= (1 << BIT);
  }
}
void new_digitalWrite(uint8_t BIT, uint8_t PIN, bool value)
{
  if (value == true) {PIN |= (1 << BIT);}
  else if (value == false) {PIN &= ~(1 << BIT);}
}

bool new_digitalRead(uint8_t BIT, uint8_t PORT)
{
  return (PORT & (1 << BIT)) == true;
}




static void SR_button_ISR(void)
{
  SR_button_pending = true;
}

static void timer_up_button_ISR(void)
{
  timer_up_button_pending = true;
}
static void timer_down_button_ISR(void)
{
  timer_down_button_pending = true;
}


//contains the main code for a timer start/reset
void timer_SR_helper(void)
{
  //the function can be locked and the time since the last call tracked
  //This is to avoid the 'bounce' effect; the period before a button is fully pushed and after fully released can
  //cause fluctioations in the output current, which will be taken as pin changes. 

  static unsigned long last_push = 0;
  static bool locked = false;



  //unlock the reset function once 200ms has passed since that last push
  if (locked && ((millis() - last_push) > bounce_buffer))
  {
    locked = false;
  }
  //The main part of the function: lock the function, track the time of this push and signal a start reset
  if (SR_button_pending && !locked)
  {
    locked = true;
    signal_start_reset();
    last_push = millis();
    SR_button_pending = false;
    LOG_TM_BUTTONS_EVENT(TIMER_SR_PUSH, millis(), 0);

  }
  //if a push has happened while the funtion is locked (within 200ms of the last push), interperet it as a bouce change and
  //disregaurd
  else if (SR_button_pending && locked)
  {
    SR_button_pending = false;
  }
  //note: all three button handler functions have this same bounce handling logic, so it will only be described here.
}

void timer_up_button_helper(void)
{
  static unsigned long last_push = 0;
  static bool locked = false;

  
  //unlock after 200ms
  if(locked and ((millis() - last_push) > bounce_buffer))
  {
    locked = false;
  }

  //lock the function and service the interrupt signal, block requests under 200ms apart
  if (!locked && timer_up_button_pending)
  {
    locked = true;
    last_push = millis();
    timer_up_button_pending = false;
    if(is_timer_expired()) {signal_start_reset();}
    increment_timer();
    LOG_TM_BUTTONS_EVENT(TIMER_UP_PUSH, millis(), get_timer_value());
  }
  else if (locked && timer_up_button_pending) {timer_up_button_pending = false;}
}




void timer_down_button_helper(void)
{
  static unsigned long last_push = 0;
  static bool locked = false;

  if (!locked && timer_down_button_pending)
  {
    locked = true;
    last_push = millis();
    timer_down_button_pending = false;
    if(is_timer_expired()) {signal_start_reset();}
    decrement_timer();
    LOG_TM_BUTTONS_EVENT(TIMER_DOWN_PUSH, millis(), get_timer_value());

    
  }
  else if (locked && timer_down_button_pending) {timer_down_button_pending = false;}


  if(locked && ((millis() - last_push) > bounce_buffer))
  {
    locked = false;
  }
}






static void (*tm_dn_btn_ISR_ptr) (void) = timer_down_button_ISR;
static void (*tm_sr_btn_ISR_ptr) (void) = SR_button_ISR;
static void (*tm_up_btn_ISR_ptr)(void) = timer_down_button_ISR;
//hi


void configure_timer_buttons(void)
{
  pinMode(timer_up_button_pin, INPUT_PULLUP);
 // new_pinMode(timer_up_button_BIT, timer_up_button_PORT, timer_up_button_DDR, 2);
  pinMode(timer_down_button_pin, INPUT_PULLUP);
  //new_pinMode(timer_down_button_BIT, timer_down_button_PORT, timer_down_button_DDR, 2);
  pinMode(timer_SR_button, INPUT_PULLUP);
  //new_pinMode(timer_SR_button_PIN, timer_SR_button_PORT, timer_SR_button_DDR, 2);

  
  //new_attachPCINT(timer_up_button_PCMSK, timer_up_button_PCINT, tm_up_btn_ISR_ptr, 1);
  //new_attachPCINT(timer_down_button_PCMSK, timer_up_button_PCINT, tm_dn_btn_ISR_ptr, 1);
  //new_attachPCINT(timer_SR_button_PCMSK, timer_SR_button_PCINT, tm_sr_ISR_ptr, 1);
  attachPCINT(up_button_PCINT, timer_up_button_ISR, FALLING);
  attachPCINT(down_button_PCINT, timer_down_button_ISR, FALLING);
  attachPCINT(SR_button_PCINT, SR_button_ISR, FALLING);

  LOG_TM_BUTTONS_EVENT(TIMER_BUTTONS_CONFIGURED, millis(), 0);
}
