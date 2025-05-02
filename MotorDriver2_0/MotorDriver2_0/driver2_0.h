/*
 * driver2_0.h
 *
 * Created: 5/3/2025 12:24:36 AM
 *  Author: adhee
 */ 


#ifndef DRIVER2_0_H_
#define DRIVER2_0_H_


/*
 * StepperDriver.h
 * Created: 4/13/2025 5:49:38 PM
 * Author: adheesha
 * 
 * Specifications:
 * - Pulses per revolution: 800
 * - Maximum frequency: 30kHz
 * - Motor revolutions ratio: 1:100 
 */
#define F_CPU 16000000UL // CPU frequency (16 MHz)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Global variables for pulse counting and control
volatile uint32_t pulseCount1 = 0; // Counter for motor 1 pulses
volatile uint32_t pulseCount2 = 0; // Counter for motor 2 pulses
uint32_t targetPulses1;            // Target pulses for motor 1
uint32_t targetPulses2;            // Target pulses for motor 2
uint16_t pRev = 1000;               // Pulses per revolution
uint16_t current_x=0;
uint16_t current_y=0;
/**
 * @brief Initializes timer and GPIO settings for stepper control
 */
void timer_setup() {
    // Configure output pins for motor control
    DDRB |= (1 << DDB5);  // Set PB5 (OC1A) as output for motor 1
    DDRC |= (1 << DDC6);  // Set PC6 (OC3A) as output for motor 2

    // Configure Timer1 for CTC (Clear Timer on Compare) mode
    TCCR1A = 1 << COM1A0;                 // Toggle OC1A on compare match
    TCCR1B = (1 << WGM12) | (1 << CS10);  // CTC mode, no prescaling

    // Configure Timer3 for CTC mode
    TCCR3A = 1 << COM3A0;                 // Toggle OC3A on compare match
    TCCR3B = (1 << WGM32) | (1 << CS30);  // CTC mode, no prescaling

    // LED and debug setup
    DDRC |= (1 << DDC7);      // Set PC7 as output for LED indicator
    DDRD |= (1 << DDD6);      // Set PD6 as output for another LED/debug
    PORTC &= ~(1 << PORTC7);  // Turn LED off initially
    
    // LED blink sequence for initialization indication
    _delay_ms(1000);
    PORTC ^= (1 << PORTC7);   // Toggle LED
    _delay_ms(500);
    PORTC ^= (1 << PORTC7);   // Toggle LED back
}

/**
 * @brief Generates pulses for stepper motors with specified frequencies and counts
 * @param f1 Frequency for motor 1 (Hz)
 * @param f2 Frequency for motor 2 (Hz)
 * @param c1 Pulse count for motor 1
 * @param c2 Pulse count for motor 2
 */
void generate_pulses(uint32_t f1, uint32_t f2, uint32_t c1, uint32_t c2) {
    // Set target pulse counts
    targetPulses1 = 2*c1;
    targetPulses2 = 2*c2;
    
    // Configure Timer1 for motor 1 if frequency > 100Hz
    if (f1 > 100) {
        OCR1A = F_CPU /(f1 * 2) -1;  // Calculate compare value for desired frequency
        TIMSK1 |= (1 << OCIE1A);   // Enable compare interrupt
    } else {
        // Disable Timer1 if frequency is too low
        OCR1A = 0;
        TCCR1B &= ~(1 << CS10);    // Stop timer
        TIMSK1 &= ~(1 << OCIE1A);   // Disable interrupt
    }

    _delay_us(15);  // Small delay to ensure proper timing

    // Configure Timer3 for motor 2 if frequency > 100Hz
    if (f2 > 100) {
        OCR3A = F_CPU /(f2 * 2) -1;  // Calculate compare value for desired frequency
        TIMSK3 |= (1 << OCIE3A);   // Enable compare interrupt
    } else {
        // Disable Timer3 if frequency is too low
        OCR3A = 0;
        TCCR3B &= ~(1 << CS30);    // Stop timer
        TIMSK3 &= ~(1 << OCIE3A);   // Disable interrupt
    }

    sei();  // Enable global interrupts
}

/**
 * @brief Timer1 compare match interrupt service routine for motor 1
 */
ISR(TIMER1_COMPA_vect) {
    pulseCount1++;  // Increment pulse counter
    
    // Check if target pulse count reached
    if (pulseCount1 >= targetPulses1) {
        PORTD |= (1 << PORTD6);  // Set PD6 (indicate completion)
        TCCR1B &= ~(1 << CS10);   // Stop Timer1
    }
}

/**
 * @brief Timer3 compare match interrupt service routine for motor 2
 */
ISR(TIMER3_COMPA_vect) {
    pulseCount2++;  // Increment pulse counter
    
    // Check if target pulse count reached
    if (pulseCount2 >= targetPulses2) {
        TCCR3B &= ~(1 << CS30);  // Stop Timer3
        PORTC |= (1 << PORTC7);   // Set PC7 (indicate completion)
    }
}

/**
 * @brief Moves stepper motors to specified target coordinates from current coordinates
 * @param target_x Target X position (in mm)
 * @param target_y Target Y position (in mm)
 * @param current_x Current X position (in mm)
 * @param current_y Current Y position (in mm)
 * @param enable_pin_enabled Flag to control enable pin (1 = enable, 0 = disable)
 */
void move_XY(uint16_t target_x, uint16_t target_y,uint8_t enable_pin_enabled) {
    
    // Configure direction and enable pins
    DDRD |= (1 << DDD0) | (1 << DDD4) | (1 << DDD2)| (1 << DDD3); // Set DIR_X, DIR_Y, EN as outputs
    // Control enable pin
    if (enable_pin_enabled) {
	    PORTD &= ~(1 << PORTD2); // Active low enable
	    PORTD &= ~(1 << PORTD3); //d3 enb d4 dir
	    } else {
	    PORTD |= (1 << PORTD2);  // Disable drivers
	    PORTD |= (1 << PORTD3);
    }
    _delay_ms(250);
    // Calculate movement distances
    uint16_t dx = target_x - current_x;
    uint16_t dy = target_y - current_y;
    
    // Set direction pins based on movement direction
    if (dx >= 0) {
        PORTD |= (1 << PORTD4);  // X direction forward
    } else {
        PORTD &= ~(1 << PORTD4); // X direction reverse
        dx = -dx; // Make distance positive
    }
    
    if (dy >= 0) {
        PORTD |= (1 << PORTD4);  // Y direction forward
    } else {
        PORTD &= ~(1 << PORTD4); // Y direction reverse
        dy = -dy; // Make distance positive
    }
    
    // Calculate required pulses (convert mm to steps)
    uint32_t x_pulses = (uint32_t)(dx * pRev);
    uint32_t y_pulses = (uint32_t)(dy * pRev);
    
    // Determine frequencies for constant velocity (adjust as needed)
    uint32_t base_freq = 10000; // Base frequency in Hz
    uint32_t x_freq = base_freq;
    uint32_t y_freq = base_freq;
    
    // Simple speed adjustment based on dominant axis (for constant velocity)
    if (x_pulses > y_pulses && y_pulses > 0) {
        y_freq = base_freq * y_pulses / x_pulses;
    } 
    else if (y_pulses > x_pulses && x_pulses > 0) {
        x_freq = base_freq * x_pulses / y_pulses;
    }
	_delay_us(5);
    
    // Generate pulses (minimum frequency of 100Hz)
    generate_pulses((x_freq < 100) ? 0 : x_freq, 
                   (y_freq < 100) ? 0 : y_freq, 
                   x_pulses, 
                   y_pulses);
	}

#endif /* DRIVER2_0_H_ */