The ATMega328 oven controller prototype by Mathew. 

More detailed descriptions of individual code modules inside .cpp files

HARDWARE MODULES:
-Arduino UNO (ATMega328)
-Dallas DS18B20 Temperature Sensor
-MQ2 sensor (pin change interrupt)
-3 Push Buttons (pin change interrupt)
-TM1637 Display
-Photoresistor (door, external interrupt)
-Relay
-Buzzer
-AND gate
-Potentiometer

-----------------------------------------------LIBRARIES-----------------------------------------------
This prototype is wired with an Arduino Uno. The code implemented uses some of the provided higher level arduino.h abstractions
like pinMode and digitalRead in the initial setup phase, however post-setup code avoids these to improve performance during run time.
The arduino.h abstractions require a bit more time and overhead i.e. mapping a pin numbers to correct port registers/bit positions,
checking and modifying relevent register configurations before performing an operation i.e. analogRead(), rather than simply writing to
the start conversion and ADC multiplexor bits.
The advantage of the Arduino library and environment is it sits on top of the AVR toolchain, so all code that invokes direct
AVR instructions will work in a normal arduino IDE without extra setup.

The program does rely on OneWire.h and DallasTemperature.h for the temperature sensor. Note that this temp sensor was only chosen
for its wider range of values. A temp sensor with a simpler interface i.e. analog output pin read through ADC would work just as well.
The TM1637 display code relies on DIYables_4Digit7Segment_TM1637.h.
All three of these libraries are available through the Arduino IDE library manager.
The good news is these sensor libraries both implement digital serial protocols, so the ADC is only needed to service one hardware 
module - the potentiometer. 


----------------------------------------------EVENT MONITORING--------------------------------------------
The serial output is configurable through the monitor_log.h file. There are three types of output events: EVENT, DEBUG and ERROR.
//#define DEBUG_MODE
#define EVENT_MODE
#define ERROR_MODE

debug events are disabled by default, simply uncomment #define DEBUG_MODE to enable it. Similarly disable any of the other two
by commenting out their #define lines in monitor_log.h. You could also customise which modules generate events by selectivley
defining ---_MODE in individual cpp files, or in a .h file if you want events related a module and all its dependant modules.

------------------------------------------------SIMULATORS------------------------------------------------
MQ2 sensor in simulators:
note that tinkercad and wokwi both have a quirk with the MQ2 sensor where its digital output pin starts LOW when the simulation
begins, regardless of what the ppm slider is set to beforehand. You have to move the slider for it to start outputting correctly.
For this reason the code ignores the MQ2 pin for 10 seconds after starting. Coincidentally, MQ2 sensor instructions specify a
20 second warm up period before sensor readings can be considered accurate, so this code feature is not too far out of place.

Temp sensors in simulators:
note that none of the temperature sensors provided by either tinkercad or wokwi are capable of sensing anything close to an
actual oven temperature (around 150-400 degrees celcius). For this reason the final return value from get_oven_temp_celcius(); adds
205 before returning. This is purley for the purpose of demonstration to simulate a real oven temperature.

---------------------------------------------DATA TYPES-----------------------------------------------
The ATMega328 is an 8-bit architecture microcontroller, so operations on larger types will require two or more clock cycles.
This program aims to use 8-bit types when possible. There are points where data resolution is sacrificed to achieve this, for example
the event struct that makes up the event queue has an 8 bit field 'value'. However some events need to pass a value larger than
8-bit, so they scale down their value with bitwise addition when passing it as an argument to log_event. Then the event queue handler
has an instruction to convert it back. This does result in some rounding imperfections, however they are not necessarily code-breaking,
just something to be aware of. 

