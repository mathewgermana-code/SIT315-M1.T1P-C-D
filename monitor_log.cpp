//the code implementation for event monitoring and serial output logic. Calls to log_event() occur throughout
//the entire program (as a macro, log_event() is abstracted to a narrower variant in each cpp file.) which creates
//and appends an instance of the Event struct from the arguments passed to the event_queue. event_queue_handler()
//is called onced every loop in main. String constants are setup to reside as part of flash memory instructions, rather than in
//dynamic memory using F(). Storing them in variables just took up too much memory.

#include <ctype.h>
#include <Arduino.h>
#include <stdint.h>
#include "monitor_log.h"



static const uint8_t queue_size = 30; //maximum 30 events can be logged each loop
typedef struct
{
    uint8_t module_ID; //enum module_ID in monitor_log.h, identifies the code file/hardware componant responsible for the event
    uint8_t type_ID; //enum type_ID: EVENT, DEBUG or ERROR
    uint8_t event_ID; //several possible events documented, event_ID used by event_queue_handler to identify which message to output
    uint32_t time_stamp; //time that the event occured
    uint8_t value; //an optional parameter only used by some event messages i.e. conversion result.
}Event;

static volatile Event event_queue[queue_size]; //the event queue, serviced and cleared once every loop


static uint8_t last_pos = 0;

//add logged event to the queue, increment last_pos until it exceeds queue_size
void log_event(uint8_t module_ID, uint8_t type_ID, uint8_t event_ID, uint32_t time_stamp, uint16_t value)
{

    if (last_pos == queue_size) {
        Serial.println("!!! EVENT QUEUE OVERFLOW !!!");
        return;
    }
    event_queue[last_pos].time_stamp = time_stamp;
    event_queue[last_pos].event_ID = event_ID;
    event_queue[last_pos].module_ID = module_ID;
    event_queue[last_pos].type_ID = type_ID;
    event_queue[last_pos].value = value;  
    ++last_pos;  
    /*
    Serial.print("LOG EVENT: last_pos after = ");
    Serial.println(last_pos);
    */
}

//The function called in the main loop, goes over all elements in the event queue and outputs their corresponding message, 
//then the queue is cleared i.e. last_pos is set back to 0.
void event_queue_handler()
{
    for(uint8_t current_pos = 0; current_pos < last_pos; ++current_pos)
    {
        static const enum type_ID valid_types[] = {DEBUG, EVENT, ERROR};
        bool valid = false;
        uint8_t type = event_queue[current_pos].type_ID;
        uint8_t event = event_queue[current_pos].event_ID;
        uint8_t value = event_queue[current_pos].value;

        switch(type)
        {
            case DEBUG:
                valid = true;
                Serial.print("[DEBUG] ");
                break;
            case EVENT:
                valid = true;
                Serial.print("[EVENT] ");
                break;
            case ERROR: 
                valid = true;
                Serial.print("[ERROR] ");
                break;
        }
        if(!valid) {continue;} //invalid type
        Serial.print(event_queue[current_pos].time_stamp);
        Serial.print("ms | ");

        switch(event)
        {
            case ADC_CONFIGURED:
                Serial.println(F("ADC config complete > start 1st conversion"));
                break;

            case BUZZER_CONFIGURED:
                Serial.println(F("Buzzer configured"));
                break;

            case DIAL_CONFIGURED:
                Serial.println(F("Dial configured"));
                break;

            case DISPLAY_CONFIGURED:
                Serial.println(F("Display configured"));
                break;

            case DOOR_CONFIGURED:
                Serial.println(F("Door sensor configured"));
                break;

            case HEAT_ELEMENT_CONFIGURED:
                Serial.println(F("Heat element configured"));
                break;

            case TIMER_COUNTER1_CONFIGURED:
                Serial.println(F("Timer counter 1 configured"));
                break;

            case OVEN_TIMER_CONFIGURED:
                Serial.println(F("Oven timer configured"));
                break;

            case MQ2_CONFIGURED:
                Serial.println(F("Smoke sensor configured"));
                break;

            case TIMER_BUTTONS_CONFIGURED:
                Serial.println(F("Timer buttons configured"));
                break;

            case ADC_START_CONVERSION:
                Serial.println(F("ADC: start conversion"));
                break;

            case ADC_CONVERT_DONE:
                Serial.print(F("ADC: conversion done, result = "));
                Serial.println(((uint16_t)value << 2));
                break;

            case BUZZER_SMOKE:
                Serial.println(F("Buzzer on: smoke alert"));
                break;

            case BUZZER_TIMER:
                Serial.println(F("Buzzer on: timer expired alert"));
                break;

            case BUZZER_OFF:
                Serial.println(F("Buzzer off"));
                break;
            
            case BUZZER_STATE_INVALID:
                Serial.print(F("Invalid buzzer state: "));
                Serial.println(value);
                break;

            case ADC_VALUE_ERROR:
                Serial.print(F("ADC value error: "));
                Serial.println((uint16_t)value << 2);
                break;

            case DIAL_TEMP_CHANGE:
                Serial.print(F("Dial: selected temp change: "));
                Serial.println(value<<1);
                break;

            case DISPLAY_UPDATED:
                Serial.print(F("Display updated: "));
                Serial.println(value);
                break;

            case DOOR_CLOSED:
                Serial.println(F("Door closed"));
                break;

            case DOOR_OPENED:
                Serial.println(F("Door opened"));
                break;

            case DOOR_TM_RESET:
                Serial.println(F("Door triggered timer reset"));
                break;

            case TEMP_EXCEEDED:
                Serial.print(F("temp sensor: "));
                Serial.print(value);
                Serial.println(F(" degrees exceeded selected temp threshold"));
                break;

            case TEMP_RECEEDED:
                Serial.print(F("temp sensor: "));
                Serial.print(value);
                Serial.println(F(" degrees fell below selected temp threshold"));
                break;

            case HEAT_ELEMENT_ON:
                Serial.println(F("Heat element on"));
                break;

            case HEAT_ELEMENT_OFF:
                Serial.println(F("Heat element off"));
                break;

            case OVEN_TIMER_STARTED:
                Serial.print(F("Oven timer started: Timer =  "));
                Serial.println(value);
                break;

            case OVEN_TIMER_STOPPED:
                Serial.print(F("Oven timer stopped/reset at: "));
                Serial.println(value);
                break;

            case MQ2_HIGH:
                Serial.println(F("Smoke sensor HIGH"));
                break;

            case MQ2_LOW:
                Serial.println(F("Smoke sensor LOW"));
                break;

            case TEMP_SENSOR_CHANGE:
                Serial.print(F("Temp sensor change: "));
                Serial.println(value << 1);
                break;

            case TIMER_UP_PUSH:
                Serial.print(F("Timer up button push: Timer = "));
                Serial.println(value);
                break;

            case TIMER_DOWN_PUSH:
                Serial.print(F("Timer down button push: Timer = "));
                Serial.println(value);
                break;

            case TIMER_SR_PUSH:
                Serial.print(F("Timer start/reset button push: Timer = "));
                Serial.println(value);
                break;
            case TEMP_SENSOR_VALUE_ERROR:
                Serial.print(F("Invalid temp sensor value: "));
                Serial.println(value << 1);
                break;
            case TEMP_SENSOR_CONVERT_DONE:
                Serial.print(F("Raw temp sensor conversion value: "));
                Serial.println(value << 1);
                break;
            case TIMER_COUNTER1_DISABLE:
                Serial.println(F("Timer Counter 1 disabled"));
                break;
            case TIMER_COUNTER1_ENABLE:
                Serial.println(F("Timer Counter 1 enabled"));
                break;
            case TIMER_STATE_CONFIG_ERROR:
                Serial.println(F("Mismatch in oven_timer.cpp timer_state & Timer Counter 1 reg configuration"));
                Serial.print(F("timer_state = "));
                Serial.println(value);
                break;
            case TIMER_VALUE_ERROR:
                Serial.print("Invalid timer value, exceeded max: timer_value = ");
                Serial.println(value);
                break;
            
            case DOOR_STATE_ERROR:
                Serial.print(F("door_open and sensor register value mismatch: door_open = "));
                Serial.println(value);
                break;
            case SIGNAL_START_RESET:
                Serial.print(F("start reset signalled, current timer_state = "));
                Serial.println(value);
                break;
        }
    }
    last_pos = 0;
}

