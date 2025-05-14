/*
 * hexsend.c
 *
 * Created: 4/23/2025 9:19:17 PM
 *  Author: HP VICTUS
 */ 
/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>


#include "Descriptors.h"

#include "LUFA/Drivers/USB/USB.h"



/* Function Prototypes: */
void SetupHardware(void);


void EVENT_USB_Device_ConfigurationChanged(void); // Write timeout exception raised in PC without this function 
void EVENT_USB_Device_ControlRequest(void);	// Cannot configure port error in PC without this function
	

/** LUFA CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = INTERFACE_ID_CDC_CCI,
				.DataINEndpoint                 =
					{
						.Address                = CDC_TX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.DataOUTEndpoint                =
					{
						.Address                = CDC_RX_EPADDR,
						.Size                   = CDC_TXRX_EPSIZE,
						.Banks                  = 1,
					},
				.NotificationEndpoint           =
					{
						.Address                = CDC_NOTIFICATION_EPADDR,
						.Size                   = CDC_NOTIFICATION_EPSIZE,
						.Banks                  = 1,
					},
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main0(void)
{
	SetupHardware();	
	sei();

for (;;)
{

	
// Send a single byte for success
	 CDC_Device_SendByte(&VirtualSerial_CDC_Interface, 0xFF);  // SUCCESS
	
	PORTC ^= (1 << PORTC7);

	// Send a single byte for error
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, 0x00);  // ERROR
	
	_delay_ms(40);
	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
}

}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{

	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);


	/* Hardware Initialization */
	USB_Init();
	
	DDRC |= (1 << DDC7);
	PORTC &= ~(1 << PORTC7); // Ensure LED is off initially
}

/** Event handler for the library USB Connection event. */

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

/** ISR to manage the reception of data from the serial port, placing received bytes into a circular buffer
 *  for later transmission to the host.
 */
/** Event handler for the CDC Class driver Line Encoding Changed event.
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
 */

