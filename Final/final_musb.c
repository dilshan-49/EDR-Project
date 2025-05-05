#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

#include "m_general.h"
#include "m_usb.h"
#include "Driver21.h"
/* Flags Used for Communication */

#define SUCCESS 0xFF
#define ERROR 0x00
#define ACK 0xAA
#define NAK 0x55
#define GENERATING 0x11
#define WAITING 0x22



void setup_hardware();
void blink_led(uint8_t count);

void echo_cordinates(uint16_t c1, uint16_t c2);

int main(void)
{
	uint16_t coordinate1 = 0;
	uint16_t coordinate2 = 0;
	
	uint8_t response = 0;
	
	m_usb_init();
	timer_setup();
	setup_hardware();
	

	while(!m_usb_isconnected()); // wait for a connection
	while(m_usb_isconnected())
	{	m_usb_tx_char(WAITING);
		if(m_usb_rx_available()==4)
		{
			// Read 16bit coordinate 1 
			pulse1=false;
			pulse2=false;
			coordinate1 = m_usb_rx_char();
			coordinate1=(coordinate1<<8)| m_usb_rx_char();
			
			// Read 16bit coordinate 2
			coordinate2 = m_usb_rx_char();
			coordinate2=(coordinate2<<8)|m_usb_rx_char();
						
			echo_cordinates(coordinate1,coordinate2);
			
			while(!m_usb_rx_available());
			response = m_usb_rx_char();
		
            if(response == ACK)
            {

	            
				m_usb_tx_char(GENERATING);
	            // Generate pulses based on counts
				pulseCount1=0;
				pulseCount2=0;
	            generate_pulses(1000,1000,coordinate1,coordinate2);
				
				while (!(pulse1 && pulse2)){
					PORTF ^= (1<<PORTF4);
					_delay_ms(100);
					};
				
				PORTF &= ((0<<PORTF0)|(0<<PORTF1)|(0<<PORTF4));
				m_usb_tx_char(SUCCESS);
				
				PORTF |= 1 << PORTF4;
				_delay_ms(200);
					
            }
			
			
            else if(response == NAK)
            {
	            // Blink LED to indicate error
	            blink_led(10);                     
            }
			
			// Flush receive buffer
			m_usb_rx_flush();
				
		}
		
		else{
			m_usb_rx_flush();
			//case of recieved bytes!=4
			PORTC^=(1<<PORTC7);
		}
			            
		// Delay 5ms
		_delay_ms(80);
	} 
	
	
	generate_pulses(1000,1000,10000,10000);
	while(!(pulse1&&pulse2)){
		PORTC^=(1<<PORTC7);
		_delay_ms(100);
	}
	blink_led(10);
} 


}

void setup_hardware(){
	//setup LED
	DDRF |= ((1<<DDF0)|(1<<DDF1)|(1<<DDF4));

	PORTF &= ((0<<PORTF0)|(0<<PORTF1)|(0<<PORTF4));
	DDRD|= (1<<DDD0)|(1<<DDD4);   // Pins for EN and DIR for PC6 PD0=D3 PD4=D4
	
}

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
	m_usb_tx_char((c1 >> 8) & 0xFF);  // Send high byte
	m_usb_tx_char(c1 & 0xFF);         // Send low byte

	// For coordinate2
	m_usb_tx_char((c2 >> 8) & 0xFF);  // Send high byte
	m_usb_tx_char(c2 & 0xFF);         // Send low byte
}
