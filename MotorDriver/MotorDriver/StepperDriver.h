/*
 * StepperDriver.h
 *
 * Created: 4/13/2025 5:49:38 PM
 *  Author: adhee
 */ 


#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define freq(ps,f) ((F_CPU / (2 * ps*f)) - 1)

volatile uint32_t pulseCount1 = 0;
volatile uint32_t pulseCount2 = 0;
uint32_t targetPulses1; // Number of pulsesgenerate to generate
uint32_t targetPulses2;
uint16_t pRev=4000;
void timer_setup() {
	DDRB |= (1 << DDB6);
	DDRC |= (1 << DDC6);
	
	TCCR1A = (1 << COM1B1)| (1 << WGM11) ;                  // Clear OC1A on compare match
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); //1 << CS10);  Prescaler = 1
	
	TCCR3A = (1 << COM3A1) | (1 << WGM31);
	TCCR3B = (1 << WGM33) | (1 << WGM32)|(1 << CS30);
	//sei();  // Enable global interrupts
	DDRC |= (1 << DDC7);  // LED indicator
	DDRD |= (1<<DDD6);
	PORTC &= ~(1 << PORTC7);  // LED off initially
	_delay_ms(1000);
	PORTC ^= (1 << PORTC7);
	_delay_ms(500);
	PORTC ^= (1 << PORTC7);
}

void generate_pulses(uint32_t f1,uint32_t f2,uint32_t c1,uint32_t c2){
	targetPulses1 = c1;
	targetPulses2 = c2;
	if (f1 != 0) {
		ICR1 = F_CPU/(f1*1);            // TOP value for 90 kHz
		OCR1B = ICR1 / 2;
		TIMSK1 |= (1 << OCIE1B);
	}
	else {
		OCR1B =0;
		TCCR1B &= ~(1 << CS12);
		TIMSK1 &= ~(1 << OCIE1B);
	}

	_delay_us(15);

	if (f2!=0) {
		ICR3 = F_CPU /(f2*1);
		OCR3A = ICR3 / 2;
		TIMSK3 |= (1 << OCIE3A);
		
	}
	else {
		OCR3A =0;
		TCCR3B &= ~(1 << CS32);
		TIMSK3 &= ~(1 << OCIE3A);
	}

	
	sei();
}



ISR(TIMER1_COMPB_vect) {
	pulseCount1++;  // Increment on each toggle (rising/falling edge)
	if ( pulseCount1>=targetPulses1){
		PORTD |= (1 << PORTD6); //PIN12
		OCR1B =0;
		TCCR1B &= ~(1 << CS10);
	}
}
ISR(TIMER3_COMPA_vect) {
	pulseCount2++;
	if (pulseCount2 >= targetPulses2) {
		OCR3A =0;
		TCCR3B &= ~(1 << CS30);  // Disable Timer3
		PORTC = (1 << PORTC7);  // PIN 13
	}
}
void move_XY(float x, float y) {
	
	uint32_t x_pulse = (uint32_t)(x*pRev/8);
	uint32_t y_pulse = (uint32_t)(y*pRev/8);
	generate_pulses(0,0,x_pulse,y_pulse);
	


}



#endif /* STEPPERDRIVER_H_ */