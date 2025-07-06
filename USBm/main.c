/*
 * USBm.c
 *
 * Created: 5/14/2025 11:25:57 PM
 * Author : HP VICTUS
 */ 

#include <avr/io.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#include "usb/usb.h"
#include "stepperDriver/StepperDriver2_0.h"
/* Flags Used for Communication */

#define SUCCESS 0xFF
#define ERROR 0x00
#define ACK 0xAA
#define NAK 0x55
#define GENERATING 0x11
#define WAITING 0x22



// -----------------------------------------------------------------------------
// Disable JTAG to access F4-F7:
// -----------------------------------------------------------------------------





void setup_hardware();
void blink_led(uint8_t count);

void echo_cordinates(uint16_t c1, uint16_t c2);

int main(void)
{
	uint16_t coordinate1 = 0;
	uint16_t coordinate2 = 0;
	
	uint8_t response = 0;
	
	usb_init();
	driver_setup();
	
	move_XY(1000,1000);
	
}
	
/*
	while(!usb_isconnected()); // wait for a connection
	while(usb_isconnected())
	{	usb_tx_char(WAITING);
		if(usb_rx_available()==4)
		{
			// Read 16bit coordinate 1

			coordinate1 = usb_rx_char();
			coordinate1=(coordinate1<<8)| usb_rx_char();
			
			// Read 16bit coordinate 2
			coordinate2 = usb_rx_char();
			coordinate2=(coordinate2<<8)|usb_rx_char();
			
			echo_cordinates(coordinate1,coordinate2);
			
			while(!usb_rx_available());
			response = usb_rx_char();
			
			if(response == ACK)
			{

				
				usb_tx_char(GENERATING);
				// Generate pulses based on counts

				generate_pulses(1000,1000,coordinate1,coordinate2);
				
				
				PORTF &= ((0<<PORTF0)|(0<<PORTF1)|(0<<PORTF4));
				usb_tx_char(SUCCESS);
				
				PORTF |= 1 << PORTF4;
				_delay_ms(200);
				
			}
			
			
			else if(response == NAK)
			{
				// Blink LED to indicate error
				blink_led(10);
			}
			
			// Flush receive buffer
			usb_rx_flush();
			
		}
		
		else{
			usb_rx_flush();
			//case of recieved bytes!=4
			PORTC^=(1<<PORTC7);
		}
		
		// Delay 5ms
		_delay_ms(80);	

	}

}

void setup_hardware(){
	//setup LED
	DDRF |= ((1<<DDF0)|(1<<DDF1)|(1<<DDF4));

	PORTF &= ((0<<PORTF0)|(0<<PORTF1)|(0<<PORTF4));
	
	
}
*/

void blink_led(uint8_t count){
	while(count>0){
		PORTF ^=(1<<PORTF4);
		count--;
		_delay_ms(100);
	}
}

void echo_cordinates(uint16_t c1, uint16_t c2){
	_delay_ms(100);
	// For coordinate1
	usb_tx_char((c1 >> 8) & 0xFF);  // Send high byte
	usb_tx_char(c1 & 0xFF);         // Send low byte

	// For coordinate2
	usb_tx_char((c2 >> 8) & 0xFF);  // Send high byte
	usb_tx_char(c2 & 0xFF);         // Send low byte
}

