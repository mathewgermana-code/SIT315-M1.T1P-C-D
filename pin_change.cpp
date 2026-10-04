/*
#include "timer_buttons.h"
#include "pin_change.h"
#include <avr/io.h>

static volatile void (*ISR_func_ptrs_PD[8])(void);
static volatile uint8_t ISR_func_modes_PD[8];

static volatile void (*ISR_func_ptrs_PC[8])(void);
static volatile uint8_t ISR_func_modes_PC[8];

void volatile (*ISR_func_ptrs_PB[8])(void);
uint8_t volatile ISR_func_modes_PB[8];
//hi
void new_attachPCINT(uint8_t PCMSK, uint8_t PCINT, uint8_t BIT, void (*ISR_function)(void), uint8_t mode_number)
{
  //uint8_t mode_number;
  ///if (mode == "change") {mode_number = 0;}
  //else if (mode == "falling") {mode_number = 1;}
  //else if (mode == "rising") {mode_number = 2;}

  //enabled pin to trigger port change interrupt

  PCMSK |= (1 << PCINT);
  if (&PCMSK == &PCMSK2) {
    ISR_func_ptrs_PD[BIT] = ISR_function;
    ISR_func_modes_PD[BIT] = mode_number;
  }
  else if (&PCMSK == &PCMSK1){
    ISR_func_ptrs_PC[BIT] = ISR_function;
    ISR_func_modes_PC[BIT] = mode_number;
  }
  else if (&PCMSK == &PCMSK0){
    ISR_func_ptrs_PB[BIT] = ISR_function;
    ISR_func_modes_PB[BIT] = mode_number;
  }
}

void new_detachPCINT(uint8_t PCMSK, uint8_t PCINT)
{
  PCMSK &= ~(1 << PCINT);
}



static inline determine_PCINT_interrupt(uint8_t BIT, uint8_t changed, uint8_t PCMSK, uint8_t PIN, void (*ISR_func_ptrs[])(void), uint8_t ISR_func_modes[])
{
  if ((changed & (1 << BIT)) && (PCMSK & (1 << BIT))) 
  {
    if (ISR_func_modes[BIT] == 0) {(*ISR_func_ptrs[BIT])();}
    else if (ISR_func_modes[BIT] == 1 && ~(PIN & (1 << BIT))) {(*ISR_func_ptrs[BIT])();}
    else if (ISR_func_modes[BIT] == 2 && (PIN & (1 << BIT))) {(*ISR_func_ptrs[BIT])();}
  }
}

ISR(PCINT2_vect)
{
  static uint8_t old_PIN = 0;
  uint8_t changed = PIND ^ old_PIN;


  determine_PCINT_interrupt(PD7, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD6, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD5, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD4, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD3, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD2, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD1, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);
  determine_PCINT_interrupt(PD0, changed, PCMSK2, PIND, ISR_func_ptrs_PD, ISR_func_modes_PD);

  old_PIN = PIND;
}

ISR(PCINT1_vect)
{
  static uint8_t old_PIN = 0;
  uint8_t changed = PINC ^ old_PIN;
  determine_PCINT_interrupt(PD7, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD6, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD5, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD4, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD3, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD2, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD1, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);
  determine_PCINT_interrupt(PD0, changed, PCMSK1, PINC, ISR_func_ptrs_PC, ISR_func_modes_PC);

  old_PIN = PINC;

}

ISR(PCINT0_vect)
{
  static uint8_t old_PIN = 0;
  uint8_t changed = PINB ^ old_PIN;
  determine_PCINT_interrupt(PD7, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD6, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD5, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD4, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD3, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD2, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD1, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);
  determine_PCINT_interrupt(PD0, changed, PCMSK0, PINB, ISR_func_ptrs_PB, ISR_func_modes_PB);

  old_PIN = PINB;

}
*/