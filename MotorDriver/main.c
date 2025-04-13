/** 
 * @file main.c
 * @author adhee
 * @date 2025-04-12
 * @brief Main function 
 */
// can go up to 70khz pulses without having measurebale delay between to timers
 #include "StepperDriver.h"
 int main(void) {
     DDRC |= (1 << DDC7);  // LED indicator
     DDRC |= (1<<DDD6);
     PORTC &= ~(1 << PORTC7);  // LED off initially 
     PORTC ^= (1 << PORTC7);
     _delay_ms(200);
     PORTC ^= (1 << PORTC7);
     timer_setup();
     generate_pulses(2000,4000,20000,10000);
     while (1) {
     }
 }