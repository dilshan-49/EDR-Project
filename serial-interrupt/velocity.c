#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#define STEP_PIN PD2
#define DIR_PIN PD3

#define STEPS_PER_MM 100
#define ACCELERATION_MM_S2 100   // mm/s^2
#define MAX_VELOCITY_MM_S 50     // mm/s

volatile uint32_t step_count = 0;
volatile uint32_t total_steps = 0;
volatile float current_velocity = 0;
volatile float accel_steps = 0;
volatile uint8_t phase = 0; // 0: accel, 1: cruise, 2: decel

void setup_pins() {
    DDRD |= (1 << STEP_PIN) | (1 << DIR_PIN);
    PORTD &= ~(1 << STEP_PIN);
}

void setup_timer1() {
    TCCR1B |= (1 << WGM12); // CTC mode
    TIMSK1 |= (1 << OCIE1A); // Enable compare interrupt
    TCCR1B |= (1 << CS10); // No prescaling
}

void start_motion(float distance_mm) {
    total_steps = (uint32_t)(distance_mm * STEPS_PER_MM);

    float accel = ACCELERATION_MM_S2 * STEPS_PER_MM;
    float vmax = MAX_VELOCITY_MM_S * STEPS_PER_MM;

    accel_steps = (vmax * vmax) / (2 * accel);

    if (2 * accel_steps > total_steps) {
        accel_steps = total_steps / 2;
    }

    phase = 0;
    step_count = 0;
    current_velocity = 0;

    OCR1A = F_CPU / (2 * 1); // Start with dummy low frequency
    setup_timer1();
    sei();
}

void set_direction(uint8_t forward) {
    if (forward)
        PORTD |= (1 << DIR_PIN);
    else
        PORTD &= ~(1 << DIR_PIN);
}

void set_step_interval(float velocity_steps_s) {
    if (velocity_steps_s < 1) velocity_steps_s = 1;
    OCR1A = (uint16_t)(F_CPU / (2 * velocity_steps_s));
}

ISR(TIMER1_COMPA_vect) {
    // Generate step pulse
    PORTD |= (1 << STEP_PIN);
    _delay_us(5); // Pulse width
    PORTD &= ~(1 << STEP_PIN);

    step_count++;

    float accel = ACCELERATION_MM_S2 * STEPS_PER_MM;
    float vmax = MAX_VELOCITY_MM_S * STEPS_PER_MM;

    if (phase == 0) {
        current_velocity += accel / (float)(F_CPU / OCR1A);
        if (step_count >= accel_steps) {
            current_velocity = vmax;
            phase = 1;
        }
    } else if (phase == 1) {
        if (step_count >= (total_steps - accel_steps)) {
            phase = 2;
        }
    } else if (phase == 2) {
        current_velocity -= accel / (float)(F_CPU / OCR1A);
        if (current_velocity < 1 || step_count >= total_steps) {
            current_velocity = 0;
            TIMSK1 &= ~(1 << OCIE1A); // Stop timer
        }
    }

    if (current_velocity > 0)
        set_step_interval(current_velocity);
}

int main(void) {
    setup_pins();
    set_direction(1); // Set motion direction

    start_motion(100.0); // Move 100 mm

    while (1) {
        // Idle or do other stuff
    }
}