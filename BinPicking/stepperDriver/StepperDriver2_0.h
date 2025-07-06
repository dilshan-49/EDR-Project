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
#ifndef STEPPERDRIVER_H_
#define STEPPERDRIVER_H_

#define F_CPU 16000000UL  // CPU frequency (16 MHz)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

// Pin definitions for clarity
#define MOTOR1_PULSE_PIN    DDB5   // OC1A output pin for motor 1 pulse
#define MOTOR2_PULSE_PIN    DDC6   // OC3A output pin for motor 2 pulse
#define MOTOR1_DIR_PIN      DDD0   // Direction pin for motor 1
#define MOTOR2_DIR_PIN      DDD4   // Direction pin for motor 2
#define MOTOR1_ENABLE_PIN   DDD2   // Enable pin for motor 1 (active low)
#define MOTOR2_ENABLE_PIN   DDD3   // Enable pin for motor 2 (active low)



#define LIMITING_SWITCH_PIN DDD2


volatile int MOTOR1_MOVING;
volatile int MOTOR2_MOVING;

void driver_setup(void) ;

void generate_pulses(uint32_t motor1Frequency, uint32_t motor2Frequency, 
                    uint32_t m1PulseCount, uint32_t m2PulseCount) ;

void move_XY(uint16_t targetPositionX, uint16_t targetPositionY) ;

void move_Z(uint16_t distance);

#endif /* STEPPERDRIVER2_0_H_ */