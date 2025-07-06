/*
 * BinPicking.c
 *
 * Created: 5/21/2025 3:49:18 PM
 * Author : HP VICTUS
 */ 
#define F_CPU 16000000UL
#define F_USB 16000000UL

/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb/usb.h"
#include "stepperDriver/StepperDriver2_0.h"
#include "endEffector/endEffector.h"

/* Flags Used for Communication */
#define READY 0x11
#define MOVING 0x22
#define MOVED 0x44
#define PICKING 0x88
#define PICKED 0x77
#define PLACING 0x66
#define PLACED 0x99
#define PAUSED 0x55
#define INITIALIZING 0xAA
#define INITIALIZED 0xCC
#define ERROR 0x1E

#define WAITING 0xFF

/* COMMANDS FROM THE MAIN CONTROLLER */
#define MOVE 0x5A
#define PICK 0xA5
#define PLACE 0x3C
#define PAUSE 0xC3
#define INITIALIZE 0x78
#define IS_FINISHED 0xE1


/* Variable and Constants for program */
const uint16_t X_initial = 50;
const uint16_t Y_initial = 50;
int state = INITIALIZE;
uint16_t X_coordinate ;
uint16_t Y_coordinate ;

/* Function Declarations */
void setup_hardware();  // setup limiting switches, indicator lights, user control switches(shift registers), motor control pins, end effector pins 
void go_to_initialPosition();
void get_coordinates();
void blink_LED(int pin);
void off_LED(int pin);
void on_LED(int pin);



int main(void)
{
	/* Initial Setup */
	usb_init();
	setup_hardware();
	driver_setup();
	
	MOTOR1_MOVING=0;
	MOTOR2_MOVING=0;

/*	_delay_ms(3000);
	PORTD |= (1 << MOTOR1_DIR_PIN);
	PORTD |= (0 << MOTOR2_DIR_PIN);
	generate_pulses(3200,4800,24000,36000);
	while(MOTOR1_MOVING||MOTOR2_MOVING){
		PORTC^=1<<PINC7;
		_delay_ms(100);
		
	}
	_delay_ms(1000);
	
	
	PORTD |= (1 << MOTOR1_DIR_PIN);
	PORTD |= (1 << MOTOR2_DIR_PIN);
	generate_pulses(3200,4800,24000,36000);
	while(MOTOR1_MOVING||MOTOR2_MOVING){
		PORTC^=1<<PINC7;
		_delay_ms(100);
			
	}
	_delay_ms(1000);
	

*/
	while(!usb_isconnected())
		_delay_ms(20);
	
	int rx_data;	
	/* Main Loop*/
	
	/* STATE MACHINE CONFIGURATION */
    while(usb_isconnected()) 
    {
		switch (state)
		{
		case WAITING:
			rx_data=usb_rx_available();
			if (rx_data==1)
				state = usb_rx_char();
			else if (rx_data>1)
				usb_rx_flush();
			break;
		
		case MOVE:
			usb_tx_char(READY);
			state= READY;
			usb_rx_flush();
			break;
		
		case READY:
			if (usb_rx_available()==4){
				get_coordinates();
				state = MOVING;}
			else if(usb_rx_available()==1)
				state = WAITING;
			else	
				 _delay_ms(100); //Wait for the arrival of coordinates
			break;
			
		case MOVING:
			usb_tx_char(MOVING);
			move_XY(X_coordinate,Y_coordinate);
			while (MOTOR2_MOVING||MOTOR2_MOVING)
				blink_LED(PINF7);
			usb_tx_char(MOVED);
			off_LED(PINF7);
			usb_rx_flush();
			state = WAITING;
			break;
			
		case PICK:
			usb_tx_char(PICKING);
			lower_gripper();
			activate_gripper();
			lift_gripper();
			usb_tx_char(PICKED);
			state = WAITING;
			break;
			
		case PLACE;
			usb_tx_char(PLACING);
			move_XY(0,0);	// Replace with bin cordinates
			while(MOTOR1_MOVING||MOTOR2_MOVING)
				blink_LED(PINF7);
			release_gripper();
			state=WAITING;
			break;
			
		case INITIALIZE;
			usb_tx_char(INITIALIZING);
			go_to_initialPosition();
			usb_tx_char(INITIALIZED);
			usb_rx_flush();
			state = WAITING;
			
		}
		
		// Need to implement the rest of the flow related to the other states
		
		
    }
}



void blink_LED(int pin){
	PORTF^=1<<pin;
	_delay_ms(50);
	
}

void off_LED(int pin){
	PORTF&=~(1<<pin);
}

void on_LED(int pin){
	PORTF&=~(1<<pin);
}

void get_coordinates(){


}

void setup_hardware(){
	DDRC|=1<<DDC7;
}

