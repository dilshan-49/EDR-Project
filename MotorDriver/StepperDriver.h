/*
 * StepperDriver.h
 * Created: 4/13/2025 5:49:38 PM
 * Author: adheesha
 * pulses/rev - 800
 * max frequency 30khz
 * motors revolutions  ratio 1:100 
 */


#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint32_t pulseCount1 = 0;
volatile uint32_t pulseCount2 = 0;
uint32_t targetPulses1;  // Number of pulsesgenerate to generate
uint32_t targetPulses2;
uint16_t pRev = 800;
void timer_setup() {
  DDRB |= (1 << DDB5);  // Set PB6 (OC1B) as output
  DDRC |= (1 << DDC6);  // Set PC6 (OC3A) as output

  // Configure Timer1 for CTC mode, no output action
  TCCR1A = 1 << COM1A0;                 // No compare output (clear COM1B bits)
  TCCR1B = (1 << WGM12) | (1 << CS10);  // CTC mode, prescaler 1

  // Configure Timer3 for CTC mode, no output action
  TCCR3A = 1 << COM3A0;                 // No compare output (clear COM3A bits)
  TCCR3B = (1 << WGM32) | (1 << CS30);  // CTC mode, prescaler 1

  // LED and debug setup
  DDRC |= (1 << DDC7);      // LED indicator (PC7)
  DDRD |= (1 << DDD6);      // Another LED or output (PD6)
  PORTC &= ~(1 << PORTC7);  // LED off initially
  _delay_ms(1000);
  PORTC ^= (1 << PORTC7);
  _delay_ms(500);
  PORTC ^= (1 << PORTC7);
}

void generate_pulses(uint32_t f1, uint32_t f2, uint32_t c1, uint32_t c2) {
  targetPulses1 = c1;
  targetPulses2 = c2;
  if (f1 > 100) {
    OCR1A = F_CPU / (f1 * 2);  // TOP value for 90 kHz
    TIMSK1 |= (1 << OCIE1A);
  } else {
    OCR1B = 0;
    TCCR1B &= ~(1 << CS10);
    TIMSK1 &= ~(1 << OCIE1A);
  }

  _delay_us(15);

  if (f2 > 100) {
    OCR3A = F_CPU / (f2 * 2);
    TIMSK3 |= (1 << OCIE3A);

  } else {
    OCR3A = 0;
    TCCR3B &= ~(1 << CS30);
    TIMSK3 &= ~(1 << OCIE3A);
  }


  sei();
}



ISR(TIMER1_COMPA_vect) {
  pulseCount1++;  // Increment on each toggle (rising/falling edge)
  if (pulseCount1 >= targetPulses1) {
    PORTD |= (1 << PORTD6);  //PIN12
    //OCR1B =0;
    TCCR1B &= ~(1 << CS10);
  }
}
ISR(TIMER3_COMPA_vect) {
  pulseCount2++;
  if (pulseCount2 >= targetPulses2) {
    //OCR3A =0;
    TCCR3B &= ~(1 << CS30);  // Disable Timer3
    PORTC = (1 << PORTC7);   // PIN 13
  }
}
void move_XY(float x, float y) {

  uint32_t x_pulse = (uint32_t)(x * pRev / 8);
  uint32_t y_pulse = (uint32_t)(y * pRev / 8);
  generate_pulses(0, 0, x_pulse, y_pulse);
}



#endif /* STEPPERDRIVER_H_ */