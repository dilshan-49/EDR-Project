#ifndef DRIVER3_0_H_
#define DRIVER3_0_H_

/*
 * StepperDriver.h
 * Created: 4/13/2025 5:49:38 PM
 * Author: adheesha
 * Modified: 5/3/2025
 * 
 * Specifications:
 * - Pulses per revolution: 1000
 * - Maximum frequency: 30kHz
 * - Motor revolutions ratio: 1:100 
 */

#define F_CPU 16000000UL  // CPU frequency (16 MHz)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Pin definitions for clarity
#define MOTOR1_PULSE_PIN    PB5   // OC1A output pin for motor 1 pulse
#define MOTOR2_PULSE_PIN    PC6   // OC3A output pin for motor 2 pulse
#define STATUS_LED_PIN      PC7   // Status LED pin
#define DEBUG_LED_PIN       PD6   // Debug LED pin
#define MOTOR1_DIR_PIN      PD0   // Direction pin for motor 1
#define MOTOR2_DIR_PIN      PD4   // Direction pin for motor 2
#define MOTOR1_ENABLE_PIN   PD2   // Enable pin for motor 1 (active low)
#define MOTOR2_ENABLE_PIN   PD3   // Enable pin for motor 2 (active low)

// Global variables for pulse counting and control
volatile uint32_t g_motor1PulseCount = 0;   // Counter for motor 1 pulses
volatile uint32_t g_motor2PulseCount = 0;   // Counter for motor 2 pulses
uint32_t g_motor1TargetPulses;              // Target pulses for motor 1
uint32_t g_motor2TargetPulses;              // Target pulses for motor 2
uint16_t g_pulsesPerRevolution = 1000;      // Pulses per revolution
uint16_t g_currentPositionX = 0;            // Current X position in mm
uint16_t g_currentPositionY = 0;            // Current Y position in mm

/**
 * @brief Initializes timer and GPIO settings for stepper control
 */
void timer_setup(void) {
    // Configure output pins for motor control
    DDRB |= (1 << MOTOR1_PULSE_PIN);  // Set motor 1 pulse pin as output
    DDRC |= (1 << MOTOR2_PULSE_PIN);  // Set motor 2 pulse pin as output

    // Configure Timer1 for CTC (Clear Timer on Compare) mode
    TCCR1A = (1 << COM1A0);                 // Toggle OC1A on compare match
    TCCR1B = (1 << WGM12) | (1 << CS10);    // CTC mode, no prescaling

    // Configure Timer3 for CTC mode
    TCCR3A = (1 << COM3A0);                 // Toggle OC3A on compare match
    TCCR3B = (1 << WGM32) | (1 << CS30);    // CTC mode, no prescaling

    // LED and debug setup
    DDRC |= (1 << STATUS_LED_PIN);          // Set status LED pin as output
    DDRD |= (1 << DEBUG_LED_PIN);           // Set debug LED pin as output
    PORTC &= ~(1 << STATUS_LED_PIN);        // Turn status LED off initially
    
    // LED blink sequence for initialization indication
    _delay_ms(1000);
    PORTC ^= (1 << STATUS_LED_PIN);         // Toggle status LED
    _delay_ms(500);
    PORTC ^= (1 << STATUS_LED_PIN);         // Toggle status LED back
}

/**
 * @brief Generates pulses for stepper motors with specified frequencies and counts
 * @param motor1Frequency Frequency for motor 1 (Hz)
 * @param motor2Frequency Frequency for motor 2 (Hz)
 * @param motor1PulseCount Pulse count for motor 1
 * @param motor2PulseCount Pulse count for motor 2
 */
void generate_pulses(uint32_t motor1Frequency, uint32_t motor2Frequency, 
                    uint32_t motor1PulseCount, uint32_t motor2PulseCount) {
    // Set target pulse counts (multiplied by 2 due to toggle mode)
    g_motor1TargetPulses = 2 * motor1PulseCount;
    g_motor2TargetPulses = 2 * motor2PulseCount;
    
    // Configure Timer1 for motor 1 if frequency > 100Hz
    if (motor1Frequency > 100) {
        OCR1A = F_CPU / (motor1Frequency * 2) - 1;  // Calculate compare value for desired frequency
        TIMSK1 |= (1 << OCIE1A);                    // Enable compare interrupt
    } else {
        // Disable Timer1 if frequency is too low
        OCR1A = 0;
        TCCR1B &= ~(1 << CS10);                     // Stop timer
        TIMSK1 &= ~(1 << OCIE1A);                   // Disable interrupt
    }

    _delay_us(15);  // Small delay to ensure proper timing

    // Configure Timer3 for motor 2 if frequency > 100Hz
    if (motor2Frequency > 100) {
        OCR3A = F_CPU / (motor2Frequency * 2) - 1;  // Calculate compare value for desired frequency
        TIMSK3 |= (1 << OCIE3A);                    // Enable compare interrupt
    } else {
        // Disable Timer3 if frequency is too low
        OCR3A = 0;
        TCCR3B &= ~(1 << CS30);                     // Stop timer
        TIMSK3 &= ~(1 << OCIE3A);                   // Disable interrupt
    }

    sei();  // Enable global interrupts
}

/**
 * @brief Timer1 compare match interrupt service routine for motor 1
 */
ISR(TIMER1_COMPA_vect) {
    g_motor1PulseCount++;  // Increment pulse counter
    
    // Check if target pulse count reached
    if (g_motor1PulseCount >= g_motor1TargetPulses) {
        TCCR1B &= ~(1 << CS10);             // Stop Timer1
    }
}

/**
 * @brief Timer3 compare match interrupt service routine for motor 2
 */
ISR(TIMER3_COMPA_vect) {
    g_motor2PulseCount++;  // Increment pulse counter
    
    // Check if target pulse count reached
    if (g_motor2PulseCount >= g_motor2TargetPulses) {
        TCCR3B &= ~(1 << CS30);             // Stop Timer31 Q1
    }
}

/**
 * @brief Moves stepper motors to specified target coordinates from current position
 * @param targetPositionX Target X position (in mm)
 * @param targetPositionY Target Y position (in mm)
 * @param enableMotors Flag to control motor enable pins (1 = enable, 0 = disable)
 */
void move_XY(uint16_t targetPositionX, uint16_t targetPositionY, uint8_t enableMotors) {
    
    // Configure direction and enable pins
    DDRD |= (1 << MOTOR1_DIR_PIN) | (1 << MOTOR2_DIR_PIN) | 
            (1 << MOTOR1_ENABLE_PIN) | (1 << MOTOR2_ENABLE_PIN);  // Set pins as outputs
    
    // Control enable pins (active low)
    if (enableMotors) {
        PORTD &= ~(1 << MOTOR1_ENABLE_PIN);  // Enable motor 1
        PORTD &= ~(1 << MOTOR2_ENABLE_PIN);  // Enable motor 2
    } else {
        PORTD |= (1 << MOTOR1_ENABLE_PIN);   // Disable motor 1
        PORTD |= (1 << MOTOR2_ENABLE_PIN);   // Disable motor 2
    }
    
    _delay_ms(250);  // Delay to ensure motors are enabled before movement
    
    // Calculate movement distances
    uint16_t distanceX = targetPositionX - g_currentPositionX;
    uint16_t distanceY = targetPositionY - g_currentPositionY;
    
    // Set direction pins based on movement direction
    if (distanceX >= 0) {
        PORTD |= (1 << MOTOR1_DIR_PIN);      // X direction forward
    } else {
        PORTD &= ~(1 << MOTOR1_DIR_PIN);     // X direction reverse
        distanceX = -distanceX;              // Make distance positive
    }
    
    if (distanceY >= 0) {
        PORTD |= (1 << MOTOR2_DIR_PIN);      // Y direction forward
    } else {
        PORTD &= ~(1 << MOTOR2_DIR_PIN);     // Y direction reverse
        distanceY = -distanceY;              // Make distance positive
    }
    
    // Calculate required pulses (convert mm to steps)
    uint32_t xPulses = (uint32_t)(distanceX * g_pulsesPerRevolution);
    uint32_t yPulses = (uint32_t)(distanceY * g_pulsesPerRevolution);
    
    // Determine frequencies for constant velocity (adjust as needed)
    uint32_t baseFrequency = 10000;          // Base frequency in Hz
    uint32_t xFrequency = baseFrequency;
    uint32_t yFrequency = baseFrequency;
    
    // Simple speed adjustment based on dominant axis (for constant velocity)
    if (xPulses > yPulses && yPulses > 0) {
        yFrequency = baseFrequency * yPulses / xPulses;
    } 
    else if (yPulses > xPulses && xPulses > 0) {
        xFrequency = baseFrequency * xPulses / yPulses;
    }
    
    _delay_us(5);  // Small delay for stability
    
    // Generate pulses (minimum frequency of 100Hz)
    generate_pulses(
        (xFrequency < 100) ? 0 : xFrequency, 
        (yFrequency < 100) ? 0 : yFrequency, 
        xPulses, 
        yPulses
    );
    
    // Update current position
    g_currentPositionX = targetPositionX;
    g_currentPositionY = targetPositionY;
}

#endif /* DRIVER3_0_H_ */