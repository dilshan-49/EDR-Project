#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

volatile uint16_t pulseCount1 = 0;
volatile uint16_t pulseCount2 = 0;
volatile uint16_t targetPulses = 10; // Number of pulses to generate
volatile uint8_t timer1Active = 1;
volatile uint8_t timer3Active = 1;

void setPWM(int f1, int f2) {
    // Set pin directions
    DDRD |= (1 << PD6);  // Timer1 PWM output (OC1A)
    DDRB |= (1 << PB5);  // Timer3 PWM output (OC3A)
    DDRC |= (1 << PC6);  // LED indicator
    
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

}

ISR(INT0_vect) {
    PORTD ^= (1 << PD6); // Toggle LED on rising edge
    //_delay_ms(10);
    pulseCount1++;
    PORTC ^= (1 << PC7);
    if (pulseCount1 >= targetPulses) {
        TCCR1B &= ~(1 << CS12);  // Disable Timer1
        timer1Active = 0;
        OCR1A = 0;
        PORTC = (1 << PC7);
        _delay_ms(100);// Turn on LED when done
    }
}
int main(void) {
    DDRD &= ~(1 << PD2); // INT0 (pin 2) as input
    PORTD &= ~(1 << PD2); // No pull-up (use external pull-down)
    DDRC |= (1 << PC7);  // LED indicator
    PORTC &= ~(1 << PC7);  // LED off initially
    uint16_t f1=1;
    setPWM(f1, 2);
    uint16_t t1=(1/f1)*1000;
    _delay_ms(t1*10);
    OCR1A =0;
    OCR3A =0;
    while (1) {
}