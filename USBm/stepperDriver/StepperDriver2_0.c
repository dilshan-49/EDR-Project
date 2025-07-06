/*
 * StepperDriver2_0.c
 *
 * Created: 5/15/2025 12:33:10 AM
 *  Author: HP VICTUS
 */ 

#include "StepperDriver2_0.h"

volatile uint32_t motor1PulseCount = 0;   // Counter for motor 1 pulses
volatile uint32_t motor2PulseCount = 0;   // Counter for motor 2 pulses
volatile uint32_t motor3PulseCount = 0;

uint32_t motor1TargetPulses;              // Target pulses for motor 1
uint32_t motor2TargetPulses;              // Target pulses for motor 2
uint32_t motor3TargetPulses;
uint16_t pulsesPerRevolution = 1000;      // Pulses per revolution
uint8_t  pitch =8;                           // lead screw pitch 8mm

uint16_t currentPositionX = 0;            // Current X position in mm
uint16_t currentPositionY = 0;            // Current Y position in mm
uint16_t currentPositionZ = 0;            // Current Y position in mm

uint8_t MAX_VEL  =200;
uint8_t MAX_ACC  =50;





/**************************************************************************
 *
 *  Interrupt Service Routines
 *
 **************************************************************************/

ISR(TIMER1_COMPA_vect) {
	motor1PulseCount++;
	if (motor1PulseCount >= motor1TargetPulses) {
		TCCR1B &= ~(1 << CS10);             // Stop Timer1
		motor1PulseCount = 0;
	}
}

ISR(TIMER3_COMPA_vect) {
	motor2PulseCount++;
	if (motor2PulseCount >= motor2TargetPulses) {
		TCCR3B &= ~(1 << CS30);             // Stop Timer3
		motor2PulseCount = 0;
	}
}

ISR(INT0_vect) {
	motor3PulseCount++;  // Increase count (can be modified for direction)
}





/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

void driver_setup(void) {
	// Configure output pins for motor control
	DDRB |= (1 << MOTOR1_PULSE_PIN) | (1<< ENDEFFECTOR_DIR2) | (1<< ENDEFFECTOR_PWM);
	DDRC |= (1 << MOTOR2_PULSE_PIN);
	DDRD |= (1<< ENDEFFECTOR_DIR1);
	TCCR1A = (1 << COM1A0);                 // Toggle OC1A on compare match
	TCCR1B = (1 << WGM12) | (1 << CS10);    // CTC mode, no prescaling

	TCCR3A = (1 << COM3A0);                 // Toggle OC3A on compare match
	TCCR3B = (1 << WGM32) | (1 << CS30);    // CTC mode, no prescaling

	DDRD |= (1 << MOTOR1_DIR_PIN) | (1 << MOTOR2_DIR_PIN) | (1 << MOTOR1_ENABLE_PIN) | (1 << MOTOR2_ENABLE_PIN);  // Set pins as outputs

	PORTD &= ~(1 << MOTOR1_ENABLE_PIN);  // Enable motor 1
	PORTD &= ~(1 << MOTOR2_ENABLE_PIN);  // Enable motor 2
	_delay_ms(250);

	TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);           // Fast PWM, non-inverting
	TCCR0B = (1 << WGM02) | (1 << CS01); // Prescaler 8   need to verify that 7.8khz ok for the motor
	OCR0A = 0;

}


void generate_pulses(uint32_t motor1Frequency, uint32_t motor2Frequency,
uint32_t m1PulseCount, uint32_t m2PulseCount) {

	motor1TargetPulses = 2 * m1PulseCount;
	motor2TargetPulses = 2 * m2PulseCount;

	if (motor1Frequency > 100) {
		OCR1A = F_CPU / (motor1Frequency * 2) - 1;  // Calculate compare value for desired frequency
		TIMSK1 |= (1 << OCIE1A);					// Enable compare interrupt
		TCCR1B |= (1 << CS10);
		
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

void move_Z(uint16_t distance){
	;
}




