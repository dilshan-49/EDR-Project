/** 
 * @file main.c
 * @author adhee
 * @date 2025-04-12
 * @brief Main function 
 */
 #define F_CPU 16000000UL
 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #include <stdint.h>
 #define freq(ps,f) ((F_CPU / (2 * ps*f)) - 1)

 volatile uint32_t pulseCount1 = 0;
 volatile uint32_t pulseCount2 = 0;
 uint32_t targetPulses1 ; // Number of pulsesgenerate to generate
 uint32_t targetPulses2 ;
 uint16_t pRev=4000;
 void timer_setup() {
     DDRB |= (1 << PB6);  
     DDRC |= (1 << PC6);
 
     TCCR1A = (1 << COM1A1)| (1 << WGM11) ;                  // Clear OC1A on compare match
     TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS12); //1 << CS10);  Prescaler = 1
 
     TCCR3A = (1 << COM3A1) | (1 << WGM31);
     TCCR3B = (1 << WGM33) | (1 << WGM32)|(1 << CS32);
     //sei();  // Enable global interrupts
 }
 
 void generate_pulses(uint32_t f1,uint32_t f2,uint32_t c1,uint32_t c2){

    if (f1 != 0) {
        ICR1 = F_CPU/(256*f1);            // TOP value for 90 kHz
        OCR1A = ICR1 / 2;  
        TIMSK1 |= (1 << OCIE1B);
    }
    else {
        OCR1B =0;
        TCCR1B &= ~(1 << CS12);
        TIMSK1 &= ~(1 << OCIE1B);
    }

     _delay_us(15);

    if (f2!=0) {
     ICR3 = F_CPU / (256*f2);
     OCR3A = ICR3 / 2;  
     TIMSK3 |= (1 << OCIE3A);
    }
    else {
        OCR3A =0;
        TCCR3B &= ~(1 << CS32);
        TIMSK3 &= ~(1 << OCIE3A);
    }

     targetPulses1 = c1;
     targetPulses2 = c2;
     sei();
 }
     
void move_XY(float x, float y) {
    
    uint32_t x_pulse = (uint32_t)(x*pRev/8);
    uint32_t y_pulse = (uint32_t)(y*pRev/8);
    generate_pulses(0,0,x_pulse,y_pulse);
 


}  


 ISR(TIMER1_COMPB_vect) {
     pulseCount1++;  // Increment on each toggle (rising/falling edge)
     if ( pulseCount1>=targetPulses1){
         PORTD |= (1 << PC6); //PIN12
         OCR1B =0;
         TCCR1B &= ~(1 << CS12);
     }
 }
 ISR(TIMER3_COMPA_vect) {
     pulseCount2++;
     if (pulseCount2 >= targetPulses2) {
         OCR3A =0;
         TCCR3B &= ~(1 << CS32);  // Disable Timer3
         PORTC = (1 << PC7);  // PIN 13
     }
 }
 int main(void) {
     DDRC |= (1 << PC7);  // LED indicator
     DDRC |= (1<<PD6);
     PORTC &= ~(1 << PC7);  // LED off initially
     PORTC ^= (1 << PC7);
     _delay_ms(200);
     PORTC ^= (1 << PC7);
     timer_setup();
     generate_pulses(2,4,20,10);
     while (1) {
     }
 }