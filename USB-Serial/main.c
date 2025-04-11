

/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
	
#include "Descriptors.h"

#include "LUFA/Drivers/Peripheral/Serial.h"
#include "LUFA/Drivers/Misc/RingBuffer.h"
#include "LUFA/Drivers/USB/USB.h"



/* Function Prototypes: */
void SetupHardware(void);


void EVENT_USB_Device_ConfigurationChanged(void); // Write timeout exception raised in PC without this function 
void EVENT_USB_Device_ControlRequest(void);	// Cannot configure port error in PC without this function
	
	
	
	/** Circular buffer to hold data from the host before it is sent to the device via the serial port. */
static RingBuffer_t USBtoUSART_Buffer;

/** Underlying data buffer for \ref USBtoUSART_Buffer, where the stored bytes are located. */
static uint8_t      USBtoUSART_Buffer_Data[128];

/** Circular buffer to hold data from the serial port before it is sent to the host. */
static RingBuffer_t USARTtoUSB_Buffer;

/** Underlying data buffer for \ref USARTtoUSB_Buffer, where the stored bytes are located. */
static uint8_t      USARTtoUSB_Buffer_Data[128];



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
int main(void)
{
	SetupHardware();

	RingBuffer_InitBuffer(&USBtoUSART_Buffer, USBtoUSART_Buffer_Data, sizeof(USBtoUSART_Buffer_Data));
	RingBuffer_InitBuffer(&USARTtoUSB_Buffer, USARTtoUSB_Buffer_Data, sizeof(USARTtoUSB_Buffer_Data));
	
	GlobalInterruptEnable();

for (;;)
{
	// Check if data is available from the PC
	int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
	if (ReceivedByte >= 0)
	{
		// Echo the received character back to the PC
		CDC_Device_SendByte(&VirtualSerial_CDC_Interface, ReceivedByte);

		// Toggle the built-in LED (PB0)

		PORTC ^= ~(1 << PORTC7); 
	}

	CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
	USB_USBTask();
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

