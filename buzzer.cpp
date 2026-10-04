#include "buzzer.h"
#include "door.h"
#include "smoke.h"
#include "oven_timer.h"
#include "monitor_log.h"

#ifdef DEBUG_MODE
  #define LOG_BUZZER_DEBUG(EVENT_ID, TIME_STAMP, VALUE) LOG_DEBUG(BUZZER, EVENT_ID, TIME_STAMP, VALUE)
#else
  #define LOG_BUZZER_DEBUG(EVENT_ID, TIME_STAMP, VALUE)

#endif

#ifdef EVENT_MODE
    #define LOG_BUZZER_EVENT(EVENT_ID, TIME_STAMP, VALUE) LOG_EVENT(BUZZER, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_BUZZER_EVENT(EVENT_ID, TIME_STAMP, VALUE)
#endif

#ifdef ERROR_MODE
    #define LOG_BUZZER_ERROR(EVENT_ID, TIME_STAMP, VALUE) LOG_ERROR(BUZZER, EVENT_ID, TIME_STAMP, VALUE)
#else
    #define LOG_BUZZER_ERROR(EVENT_ID, TIME_STAMP, VALUE)
#endif
//-----------------------------------------------------------

enum buzzer_states : uint8_t
{
  BUZZER_OFF_STATE,
  BUZZER_SMOKE_STATE,
  BUZZER_TIMER_STATE
};

//input pin for the buzzer module, used for indicating that the timer has completed,
//and for alerting that smoke is detected
static const uint8_t buzzer_pin = 7;
//buzzer frequencies for the tone() function
static const uint8_t timer_expired_tone = 500;
static const uint8_t smoke_alert_tone = 200;

void configure_buzzer(void)
{
  pinMode(buzzer_pin, OUTPUT);
  LOG_BUZZER_EVENT(BUZZER_CONFIGURED, millis(), 0);
}

//called in the main loop for determining whether the buzzer should sound or not
void determine_buzzer_state(void)
{
  uint8_t buzzer_state = BUZZER_OFF_STATE;
  static uint8_t old_buzzer_state = BUZZER_OFF_STATE;
  if (get_current_smoke_state()) {buzzer_state = BUZZER_SMOKE_STATE;}
  else if (is_timer_expired()) {buzzer_state = BUZZER_TIMER_STATE;}
  if (is_door_open()) {buzzer_state = BUZZER_OFF_STATE;}

  if(buzzer_state != old_buzzer_state)
  {
    switch (buzzer_state)
    {
      case BUZZER_SMOKE_STATE:
        //tone simply toggles the buzzer pin at the given frequency using one of the timer modules
        //no PWM required
        tone(buzzer_pin, smoke_alert_tone);
        LOG_BUZZER_EVENT(BUZZER_SMOKE, millis(), 0);
        break;
      case BUZZER_TIMER_STATE:
        tone(buzzer_pin, timer_expired_tone);
        LOG_BUZZER_EVENT(BUZZER_TIMER, millis(), 0);
        break;
      case BUZZER_OFF_STATE:
        noTone(buzzer_pin);
        LOG_BUZZER_EVENT(BUZZER_OFF, millis(), 0);
        break;


      
      default:
        noTone(buzzer_pin);
        LOG_BUZZER_ERROR(BUZZER_STATE_INVALID, millis(), buzzer_state);
        buzzer_state = BUZZER_OFF_STATE;
        break;
    }
    old_buzzer_state = buzzer_state;
  }
  
}
  