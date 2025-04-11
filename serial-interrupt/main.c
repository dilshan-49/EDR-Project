
/* Includes: */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/power.h>
	
#include "Descriptors.h"

#include "LUFA/Drivers/Peripheral/Serial.h"
#include "LUFA/Drivers/Misc/RingBuffer.h"
#include "LUFA/Drivers/USB/USB.h"
#include "LUFA/Platform/Platform.h"

/* Global Variables */
volatile uint16_t pulseCount1 = 0;
volatile uint16_t targetPulses = 0;
volatile uint8_t pulseGenerationComplete = 0;

	
static RingBuffer_t RX_Buffer;	// Circular buffer to hold data from the host before it is sent to the device via the serial port. 
static uint8_t      RX_Buffer_Data[128]; // Underlying data buffer for \ref USBtoUSART_Buffer, where the stored bytes are located.

static RingBuffer_t TX_Buffer;	// Circular buffer to hold data from the serial port before it is sent to the host.
static uint8_t      TX_Buffer_Data[128];	// Underlying data buffer for \ref USARTtoUSB_Buffer, where the stored bytes are located.

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

/* USART1 Receive Interrupt Service Routine */
ISR(USART1_RX_vect, ISR_BLOCK) {
	uint8_t ReceivedByte = UDR1;

	if ((USB_DeviceState == DEVICE_STATE_Configured) && !(RingBuffer_IsFull(&RX_Buffer))) {
		RingBuffer_Insert(&RX_Buffer, ReceivedByte);
	}
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

	RingBuffer_InitBuffer(&RX_Buffer, RX_Buffer_Data, sizeof(RX_Buffer_Data));
	RingBuffer_InitBuffer(&TX_Buffer, TX_Buffer_Data, sizeof(TX_Buffer_Data));
	
	GlobalInterruptEnable();

	for (;;)
	{
		// Check if data is available from the PC
		if (!RingBuffer_IsEmpty(&RX_Buffer))
		{	
			PORTD &= ~(1 << PORTD4); 
			// Read the target pulse count from the buffer
			uint8_t byte1 = RingBuffer_Remove(&RX_Buffer);
			uint8_t byte2 = RingBuffer_Remove(&RX_Buffer);
			targetPulses = (byte1 << 8) | byte2; // Combine two bytes into a 16-bit value

			// Start generating pulses
			pulseCount1 = 0;
			pulseGenerationComplete = 0;
			timer1_ctc(2); // Example frequency, adjust as needed
			
			while (!pulseGenerationComplete)
			{
				// Handle USB tasks to keep the connection alive
				CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
				USB_USBTask();
			}
			char message[50];
			snprintf(message, sizeof(message), "Successfully generated %u pulses\r\n", targetPulses);
			CDC_Device_SendString(&VirtualSerial_CDC_Interface, message);
			
		}

		// Handle USB tasks
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