// -----------------------------------------------------------------------------
// AVR USB communication subsystem for ATmega32u4
// version: 2.2
// same as version 2.1 built for M2 MAEVARM
// date: June 16, 2012
// authors: J. Fiene & J. Romano
// -----------------------------------------------------------------------------

#ifndef m_usb__
#define m_usb__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>


// -----------------------------------------------------------------------------
// Public functions:
// -----------------------------------------------------------------------------

// INITIALIZATION: -------------------------------------------------------------

void m_usb_init(void);	
// initialize the USB subsystem

char m_usb_isconnected(void);
// confirm that the USB port is connected to a PC


// RECEIVE: -------------------------------------------------------------------

unsigned char m_usb_rx_available(void);		   		   
// returns the number of bytes (up to 255) waiting in the receive FIFO buffer

char m_usb_rx_char(void);		   			   
// retrieve a oldest byte from the receive FIFO buffer (-1 if timeout/error)

void m_usb_rx_flush(void);		   			   
// discard all data in the receive buffer



// TRANSMIT: ------------------------------------------------------------------

char m_usb_tx_char(unsigned char c);                 
// add a single 8-bit unsigned char to the transmit buffer, return -1 if error

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// ---- OVERLOADS FOR M1 BACK COMPATIBILITY ----
#define usb_init()			m_usb_init()
#define usb_configured()	m_usb_isconnected()

#define usb_rx_available()	m_usb_rx_available()		   		   
#define usb_rx_flush()		m_usb_rx_flush()		   			   
#define usb_rx_char()		m_usb_rx_char()

#define usb_tx_char(val)	m_usb_tx_char(val)


#define m_usb_rx_ascii()	m_usb_rx_char()
#define m_usb_tx_ascii(val)	m_usb_tx_char(val)




// constants corresponding to the various serial parameters
#define USB_SERIAL_DTR			0x01
#define USB_SERIAL_RTS			0x02
#define USB_SERIAL_1_STOP		0
#define USB_SERIAL_1_5_STOP		1
#define USB_SERIAL_2_STOP		2
#define USB_SERIAL_PARITY_NONE		0
#define USB_SERIAL_PARITY_ODD		1
#define USB_SERIAL_PARITY_EVEN		2
#define USB_SERIAL_PARITY_MARK		3
#define USB_SERIAL_PARITY_SPACE		4
#define USB_SERIAL_DCD			0x01
#define USB_SERIAL_DSR			0x02
#define USB_SERIAL_BREAK		0x04
#define USB_SERIAL_RI			0x08
#define USB_SERIAL_FRAME_ERR		0x10
#define USB_SERIAL_PARITY_ERR		0x20
#define USB_SERIAL_OVERRUN_ERR		0x40

// This file does not include the HID debug functions, so these empty
// macros replace them with nothing, so users can compile code that
// has calls to these functions.


#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40
#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06
#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

#define MAX_ENDPOINT		4

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#define HW_CONFIG() (UHWCON = 0x01)

#ifdef M1
	#define PLL_CONFIG() (PLLCSR = 0x02) // fixed to 8MHz clock
#else
	#define PLL_CONFIG() (PLLCSR = 0x12) // 0001 0010 For a 16MHz clock
#endif

#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))

// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
// CDC (communication class device)
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22

#endif
