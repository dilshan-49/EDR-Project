#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "Driver21.h"



	
int main(void) {
	timer_setup();
	generate_pulses(6,6,30,30);
	
	while (1) {
		_delay_ms(100);
	}
}