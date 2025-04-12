#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint16_t pulseCount1 = 0;
volatile uint16_t pulseCount2 = 0;
volatile uint16_t targetPulses = 20; // Number of pulsesgenerate to generate

void timer1_ctc(uint16_t f1) {
	DDRB |= (1 << PORTB5);  // OC1A pin
	uint16_t ocr_val = (F_CPU / (2 * 256 * f1)) - 1;  // 399
	OCR1A = ocr_val;
	TCCR1A = (1 << COM1A0);           // Toggle OC1A on match
	TCCR1B = (1 << WGM12) | (1 << CS12);  // CTC mode, prescaler = 1
	TIMSK1 |= (1 << OCIE1A);
	sei();  // Enable global interrupts
}
ISR(TIMER1_COMPA_vect) {
	pulseCount1++;  // Increment on each toggle (rising/falling edge)
	if ( pulseCount1>=targetPulses){
		PORTC |= (1 << PORTC7);
		TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
		//TCCR1A &= ~((1 << COM1A1) | (1 << COM1A0));  // Disconnect OC1A
		PORTB &= ~(1 << PORTB5);
	}
}
void setPWM(int f1, int f2) {
	// Set pin directions
	DDRD |= (1 << PORTD6);  // Timer1 PWM output (OC1A)
	DDRB |= (1 << PORTB5);  // Timer3 PWM output (OC3A)
	DDRC |= (1 << PORTC6);  // LED indicator
	
	// Timer1 setup (for PD6)
	int top1 = (62500/f1)-1;
	TCCR1A = (1 << COM1A1)| (1 << WGM11);  // Fast PWM, TOP=ICR1
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12);  // Prescaler=256
	ICR1 = top1;
	OCR1A = ICR1/2;  // 50% duty cycle
	
	// Timer3 setup (for PB5)
	int top2 = (62500/f2)-1;
	TCCR3A = (1 << COM3A1) | (1 << WGM31);  // Fast PWM, TOP=ICR3
	TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS32);  // Prescaler=256
	ICR3 = top2;
	OCR3A = ICR3/2;  // 50% duty cycle
	
	// Enable interrupts
	//TIMSK1 |= (1 << OCIE1A);  // Timer1 Compare Match A interrupt
	//TIMSK3 |= (1 << OCIE3A);  // Timer3 Compare Match A interrupt

}

//ISR(TIMER3_COMPA_vect) {
//
//    if (timer3Active==1) {
//        pulseCount2++;
//        if (pulseCount2 >= targetPulses) {
//            TCCR3B &= ~(1 << CS32);  // Disable Timer3
//            timer3Active = 0;
//            PORTC = (1 << PC6);  // Turn on LED when done
//        }
//    }
//}

void blink_led(){
	_delay_ms(200);
	PORTD ^= (1<<PORTD4);
	
}
int main(void) {
	DDRC |= (1 << PORTC7);  // LED indicator
	DDRD |= (1 << PORTD4);
	PORTC &= ~(1 << PORTC7);  // LED off initially
	PORTC ^= (1 << PORTC7);
	_delay_ms(200);
	PORTC ^= (1 << PORTC7);
	timer1_ctc(2);
	while (1) {
		blink_led();		
	}
}