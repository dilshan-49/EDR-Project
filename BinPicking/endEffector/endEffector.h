/*
 * endEffector.h
 *
 * Created: 7/3/2025 8:59:28 PM
 *  Author: HP VICTUS
 */ 


#ifndef ENDEFFECTOR_H_
#define ENDEFFECTOR_H_

#define F_CPU 16000000UL  // CPU frequency (16 MHz)
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>


#define ENDEFFECTOR_DIR1    DDD5
#define ENDEFFECTOR_DIR2    DDB6
#define ENDEFFECTOR_PWM     DDB7
#define ENCODER_A           DDD0

#define VACUM_PUMP          DDC7
#define SOLENOID_VALVE      DDE2



void activate_gripper();
void lower_gripper();
void lift_gripper();
void release_gripper();


#endif /* ENDEFFECTOR_H_ */