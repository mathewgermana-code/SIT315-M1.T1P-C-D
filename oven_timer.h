#ifndef OVEN_TIMER_H
#define OVEN_TIMER_H
#include <Arduino.h>

void configure_oven_timer(void);

uint8_t get_timer_value(void);
void set_timer_zero(void);
void set_timer_max(void);
bool get_timer_state(void);
bool is_timer_expired(void);
void flip_timer_state(void);
uint8_t increment_timer(void);
uint8_t decrement_timer(void);

void signal_start_reset(void);
void start_reset_handler(void);


#endif