#ifndef PINS_H
#define PINS_H

#define STEPS   { 15, 22, 3, 1 }
#define DIRS    { 21, 23, 2, 0 }
#define ENABLES { 14, 14, 26, 14 }

#define THERMS  { 7, 6 } // Analogue
#define HEATERS { 13, 12 }

#define LED_PIN 27
#define LED_BLINK 4

// Incoming master-clock interrupt on D16 (PCINT16)
// Change state on this to step a drive
// #define INTERRUPT_PIN 16

#endif
