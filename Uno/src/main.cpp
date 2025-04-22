#include <Arduino.h>

// Timer pins for pulse counting
const int pulsePin1 = 5; // T1 (Timer1 external clock input)

// Variables to store pulse counts
volatile unsigned long pulseCount1 = 0;

// Cumulative pulse counts
unsigned long cumulativeCount1 = 0;
// Interval for reporting pulse counts
unsigned long previousMillis = 0;
const long interval = 1000; // Report every 1 second

void setup() {
  // Configure pulse pins as inputs with pull-up resistors
  pinMode(pulsePin1, INPUT_PULLUP);

  // Initialize Serial Monitor
  Serial.begin(9600);
  Serial.println("Starting pulse counting using hardware timers...");

  // Configure Timer1 for external clock on T1 (pin 5)
  TCCR1A = 0; // Normal mode
  TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10); // External clock source on T1 (rising edge)

}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read pulse counts from hardware timers
    unsigned long count1;
    noInterrupts(); // Temporarily disable interrupts to read timer values safely
    count1 = TCNT1; // Read Timer1 count
    TCNT1 = 0;      // Reset Timer1 count
    interrupts(); // Re-enable interrupts

    // Update cumulative counts
    cumulativeCount1 += count1;

    // Print cumulative pulse counts to Serial Monitor
    Serial.print(" Pulse Count on Pin 5 (Timer1): ");
    Serial.println(cumulativeCount1);
    Serial.print(" Frequency: ");
    Serial.println(count1);
    Serial.println("-------------------");
  }
}