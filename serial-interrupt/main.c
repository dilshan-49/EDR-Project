
/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
	
#include "Descriptors.h"

#include "LUFA/Drivers/Peripheral/Serial.h"
#include "LUFA/Drivers/Misc/RingBuffer.h"
#include "LUFA/Drivers/USB/USB.h"

/* Global Variables */
volatile uint16_t pulseCount1 = 0;
volatile uint16_t targetPulses = 0;
volatile uint8_t pulseGenerationComplete = 0;

uint8_t receivedBytes[2]; // Buffer to store the two bytes
uint8_t byteIndex = 0;

static RingBuffer_t RX_Buffer;	// Circular buffer to hold data from the host before it is sent to the device via the serial port. 
static uint8_t      RX_Buffer_Data[128]; // Underlying data buffer for \ref USBtoUSART_Buffer, where the stored bytes are located.

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



/* Function Prototypes: */

void SetupHardware(void);
void timer1_ctc(uint16_t f1);
void SetupTimer0();
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);


/* ISR */
		
ISR(TIMER1_COMPA_vect) {
	pulseCount1++;  // Increment on each toggle (rising/falling edge)
	PORTD ^= (1 << PORTD4);
	if (pulseCount1 >= targetPulses) {
		PORTC |= (1 << PORTC7);  // Turn on LED to indicate completion
		TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));  // Stop Timer1
		PORTB &= ~(1 << PORTB5);  // Turn off pulse output
		pulseGenerationComplete = 1;  // Indicate pulse generation is complete
	}
}

ISR(TIMER0_COMPA_vect)
{
    USB_USBTask(); // Call USB task periodically
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	DDRC |= (1 << DDC7);
	DDRD |= (1 << DDD4);
	PORTC &= ~(1 << PORTC7); // Ensure LED is off initially
	PORTD |= (1 << PORTD4); 
	SetupTimer0();
	RingBuffer_InitBuffer(&RX_Buffer, RX_Buffer_Data, sizeof(RX_Buffer_Data));
	
	GlobalInterruptEnable();

	for (;;)
	{
		// Check if data is available from the PC
		int16_t ReceivedByte = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		if (ReceivedByte >= 0)
		{	
			receivedBytes[byteIndex++] = (uint8_t)ReceivedByte;

			if (byteIndex == 2)
			{	
				PORTD &= ~(1 << PORTD4); 
				// Combine the two bytes into a 16-bit integer (big-endian)
				targetPulses = (receivedBytes[0] << 8) | receivedBytes[1];
				byteIndex = 0; // Reset the buffer index
	
				// Start generating pulses
				pulseCount1 = 0;
				pulseGenerationComplete = 0;
				timer1_ctc(2); // Example frequency, adjust as needed
			}
		}
			
		if (pulseGenerationComplete)
			{
				PORTC |= (1 << PORTC7); // Turn on LED to indicate completion	
				char message[50];
				snprintf(message, sizeof(message), "Successfully generated %u pulses\r\n", targetPulses);
				CDC_Device_SendString(&VirtualSerial_CDC_Interface, message);
				CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
				pulseGenerationComplete=0;
			}
			
		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
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
}



/* Interrupt Related Functions */

void timer1_ctc(uint16_t f1) {
	DDRB |= (1 << PORTB5);  // OC1A pin
	uint16_t ocr_val = (F_CPU / (2 * 256 * f1)) - 1;  // 399
	OCR1A = ocr_val;
	TCCR1A = (1 << COM1A0);           // Toggle OC1A on match
	TCCR1B = (1 << WGM12) | (1 << CS12);  // CTC mode, prescaler = 1
	TIMSK1 |= (1 << OCIE1A);
	sei();  // Enable global interrupts
}

void SetupTimer0(void)
{	uint16_t ocr_val = 472;
	OCR0A = ocr_val; // Compare value for ~30ms
    TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS02) | (1 << CS00); // Prescaler = 1024
    TIMSK0 |= (1 << OCIE0A); // Enable compare match interrupt
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
