#include "timer_buttons.h"
#include "heat_element.h"
#include "buzzer.h"
#include "display.h"
#include "door.h"
#include "smoke.h"
#include "oven_timer.h"
#include "monitor_log.h"
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/interrupt.h>
















//used for the conversion formula that converts the temp sensor input into degrees celcius
//-no longer used
//const float BETA = 3950; // should match the Beta Coefficient of the thermistor









//constants for photoresistor(light sensor)






/*The central function called in loop(), in the first 10 seconds it points to pre_MQ2_setup and 
the program is in a state where it ignores the MQ2 smoke sensing. After 10 seconds finish_MQ2_setup()
is called and post_MQ2_setup is assigned to loop_function.*/
void (*loop_function)(void);

void post_MQ2_setup()
{
  wdt_reset();
  set_heat_state(); //heat_element.cpp
  determine_buzzer_state(); //buzzer.cpp
  update_display(); //display.cpp
  timer_up_button_helper(); //timer_buttons.cpp
  timer_down_button_helper();//timer_buttons.cpp
  timer_SR_helper();//timer_buttons.cpp
  start_reset_handler();//oven_timer.cpp
  event_queue_handler();//monitor_log.cpp
}

void pre_MQ2_setup()
{
  wdt_reset();
  set_heat_state(); //heat_element.cpp
  determine_buzzer_state(); //buzzer.cpp
  update_display(); //display.cpp
  timer_up_button_helper(); //timer_buttons.cpp
  timer_down_button_helper(); //timer_buttons.cpp
  timer_SR_helper(); //timer_buttons.cpp
  start_reset_handler(); //oven_timer.cpp
  event_queue_handler(); //monitor_log.cpp
  //after 10 seconds stop ignoring MQ2, complete its setup
  if (millis() > 10000)
  {
    finish_MQ2_setup();
    loop_function = &post_MQ2_setup;
  }
}

void setup() {
  loop_function = &pre_MQ2_setup;
  Serial.begin(9600);

  
  wdt_disable();
  configure_oven_timer();
  configure_timer_buttons();
  configure_heat_element();
  configure_smoke_sensor();
  configure_buzzer();
  configure_display();
  configure_door();
  //wdt_enable(WDTO_2S);
  //configure_dial() and configure_temp_sensor() are called in configure_heat_element()
  //configure_timerCounter1() is called in configure_oven_timer()


  



}

void loop() {
  (*loop_function)();

}