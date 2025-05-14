/* Includes: */
	#include <avr/io.h>
	#include <avr/wdt.h>
	#include <avr/interrupt.h>
	#include <avr/power.h>

	#include "Descriptors.h"
	
	#include "LUFA/Drivers/USB/USB.h"
	#include "stepperDriver.h"


/* Flags Used for Communication */
	#define SUCCESS 0xFF
	#define ERROR 0x00
	#define ACK 0xAA
	#define NAK 0x55
	#define GENERATING 0x11
	#define WAITING 0x22
	
/* Function Prototypes: */
	void SetupHardware(void);
	void blink_led(uint8_t count);
	void echo_coordinates(uint16_t c1, uint16_t c2);

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

	
	
	
int main(void){
	uint16_t coordinate1 = 0;
	uint16_t coordinate2 = 0;
	bool pulse1;
	bool pulse2;
	SetupHardware();
	
    while (1)
    {
	    // Only process if device is configured and connected
	    if (USB_DeviceState == DEVICE_STATE_Configured)
	    {
		    // Send waiting status
		    CDC_Device_SendByte(&VirtualSerial_CDC_Interface, WAITING);
		    
		    // Check if exactly 4 bytes are available
		    if (CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface) == 4)
		    {
			    // Reset pulse flags
			    pulse1 = false;
			    pulse2 = false;
			    
			    // Read 16-bit coordinate 1
			    uint8_t high = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
			    uint8_t low = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
			    coordinate1 = (high << 8) | low;
			    
			    // Read 16-bit coordinate 2
			    high = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
			    low = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
			    coordinate2 = (high << 8) | low;
			    
			    // Echo coordinates back to host
			    echo_coordinates(coordinate1, coordinate2);
			    
			    // Send generating status
			    CDC_Device_SendByte(&VirtualSerial_CDC_Interface, GENERATING);
			    
			    // Generate pulses based on coordinates
			    pulseCount1 = 0;
			    pulseCount2 = 0;
			    generate_pulses(1000, 1000, coordinate1, coordinate2);
			    
			    // Wait for pulses to complete
			    while (!(pulse1 && pulse2)) {
				    PORTF ^= (1 << PORTF4);
				    _delay_ms(100);
			    }
			    
			    // Turn off LEDs
			    PORTF &= ~((1 << PORTF0) | (1 << PORTF1) | (1 << PORTF4));
			    
			    // Send success status
			    CDC_Device_SendByte(&VirtualSerial_CDC_Interface, SUCCESS);
			    
			    // Blink LED to indicate success
			    blink_led(8);
		    }
	    }
	    
	    // Run CDC tasks
	    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	    USB_USBTask();
	    
	    _delay_ms(5);
    }

}



/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
	#endif

	/* Hardware Initialization */
	USB_Init();
	
	DDRC |= (1 << DDC7);
	PORTC &= ~(1 << PORTC7); // Ensure LED is off initially
}

void echo_coordinates(uint16_t c1, uint16_t c2)
{
	_delay_ms(100);
	
	// For coordinate1
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (c1 >> 8) & 0xFF);  // Send high byte
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c1 & 0xFF);         // Send low byte
	
	// For coordinate2
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, (c2 >> 8) & 0xFF);  // Send high byte
	CDC_Device_SendByte(&VirtualSerial_CDC_Interface, c2 & 0xFF);         // Send low byte
}

void blink_led(uint8_t count)
{
	while (count > 0) {
		PORTF ^= (1 << PORTF4);
		count--;
		_delay_ms(100);
	}
}


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