/*
 * File:   motor_driver.c
 * Author: adheesha
 *
 * Created on April 7, 2025, 12:12 PM
 */
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
void setPWM(int f1,int f2){
    DDRD |= (1 << PD6); 
    DDRB |= (1 << PB1);
    DDRC |= (1 << PC6); 
    DDRB |= (1 << PB5);
    int top1= (16000000/f1)-1;
    TCCR1A = (1 << COM1A1) | (1 << WGM11); // Clear OC1A on compare match, Fast PWM (TOP=ICR1)
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // Prescaler 256, Fast PWM
    ICR1 = top1;//6249; // TOP value for 10 Hz (62,500 / 10 - 1)
    OCR1A = ICR1/2; // 50% duty cycle (half of ICR1)

    TCCR3A = (1 << COM3A1) | (1 << WGM31); // Clear OC3A on compare match, Fast PWM (TOP=ICR3)
    TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS30); // Prescaler 256, Fast PWM
    int top2= (16000000/f2)-1;
    ICR3 = top2; // TOP value for 20 Hz (62,500 / 20 - 1)
    OCR3A = ICR3/2; // 50% duty cycle (half of ICR3)                                      // 50% duty cycle (127/255 ? 50%)
    
}
void blink(){
    DDRC |= 1<<PC7;  
    while (1){
        PORTC^=1<<PC7;
        _delay_ms(300);
    }
}
int main(void) {
    setPWM(10,20);
    while (1) { 
    blink();
    }
}
