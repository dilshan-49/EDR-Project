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
#ifndef STEPPERDRIVER2_0_H_
#define STEPPERDRIVER2_0_H_

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
volatile uint32_t motor1PulseCount = 0;   // Counter for motor 1 pulses
volatile uint32_t motor2PulseCount = 0;   // Counter for motor 2 pulses
uint32_t motor1TargetPulses;              // Target pulses for motor 1
uint32_t motor2TargetPulses;              // Target pulses for motor 2
uint16_t pulsesPerRevolution = 1000;      // Pulses per revolution
uint16_t currentPositionX = 0;            // Current X position in mm
uint16_t currentPositionY = 0;            // Current Y position in mm
uint8_t pitch =8;                           // lead skrew pitch 8mm
/**
 * @brief Initializes timer and GPIO settings for stepper control
 */
void driver_setup(void) {
    // Configure output pins for motor control
    DDRB |= (1 << MOTOR1_PULSE_PIN);  
    DDRC |= (1 << MOTOR2_PULSE_PIN); 

    TCCR1A = (1 << COM1A0);                 // Toggle OC1A on compare match
    TCCR1B = (1 << WGM12) | (1 << CS10);    // CTC mode, no prescaling

    TCCR3A = (1 << COM3A0);                 // Toggle OC3A on compare match
    TCCR3B = (1 << WGM32) | (1 << CS30);    // CTC mode, no prescaling

    DDRD |= (1 << MOTOR1_DIR_PIN) | (1 << MOTOR2_DIR_PIN) | (1 << MOTOR1_ENABLE_PIN) | (1 << MOTOR2_ENABLE_PIN);  // Set pins as outputs

    PORTD &= ~(1 << MOTOR1_ENABLE_PIN);  // Enable motor 1
    PORTD &= ~(1 << MOTOR2_ENABLE_PIN);  // Enable motor 2
    _delay_ms(250);  
}

void generate_pulses(uint32_t motor1Frequency, uint32_t motor2Frequency, 
                    uint32_t motor1PulseCount, uint32_t motor2PulseCount) {

    motor1TargetPulses = 2 * motor1PulseCount;
    motor2TargetPulses = 2 * motor2PulseCount;

    if (motor1Frequency > 100) {
        OCR1A = F_CPU / (motor1Frequency * 2) - 1;  // Calculate compare value for desired frequency
        TIMSK1 |= (1 << OCIE1A); 
        TCCR1B |= (1 << CS10);
                         // Enable compare interrupt
    } else {
        OCR1A = 0;
        TCCR1B &= ~(1 << CS10);                     // Stop timer
        TIMSK1 &= ~(1 << OCIE1A);                   // Disable interrupt
    }

    _delay_us(15);  // Small delay to ensure proper timing

    if (motor2Frequency > 100) {
        OCR3A = F_CPU / (motor2Frequency * 2) - 1;  // Calculate compare value for desired frequency
        TIMSK3 |= (1 << OCIE3A);  
        TCCR3B |= (1 << CS30);                     // Enable compare interrupt
    } else {
        OCR3A = 0;
        TCCR3B &= ~(1 << CS30);                     // Stop timer
        TIMSK3 &= ~(1 << OCIE3A);                   // Disable interrupt
    }

    sei();  // Enable global interrupts
}

ISR(TIMER1_COMPA_vect) {
    motor1PulseCount++; 
    if (motor1PulseCount >= motor1TargetPulses) {
        TCCR1B &= ~(1 << CS10);             // Stop Timer1
    }
}

ISR(TIMER3_COMPA_vect) {
    motor2PulseCount++; 
    if (motor2PulseCount >= motor2TargetPulses) {
        TCCR3B &= ~(1 << CS30);             // Stop Timer3
    }
}

void move_XY(uint16_t targetPositionX, uint16_t targetPositionY) {
    // Calculate movement distances
    uint16_t distanceX = targetPositionX - currentPositionX;
    uint16_t distanceY = targetPositionY - currentPositionY;
    
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
    uint32_t xPulses = (uint32_t)(distanceX * pulsesPerRevolution / pitch);
    uint32_t yPulses = (uint32_t)(distanceY * pulsesPerRevolution / pitch);
    
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
    generate_pulses(
        (xFrequency < 100) ? 0 : xFrequency, 
        (yFrequency < 100) ? 0 : yFrequency, 
        xPulses, 
        yPulses
    );
    
    // Update current position
    currentPositionX = targetPositionX;
    currentPositionY = targetPositionY;
}

#endif /* STEPPERDRIVER2_0_H_ */