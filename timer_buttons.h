//interface for the buttons that let the user control the timer

#ifndef BUTTONS_H
#define bUTTONS_H
#include <Arduino.h>


void configure_timer_buttons(void);

void timer_SR_helper(void);

void timer_up_button_helper(void);

void timer_down_button_helper(void);

#endif