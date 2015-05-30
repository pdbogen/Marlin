#ifndef GLOBALS_H
#define GLOBALS_H

#include "Configuration.h"
#include "Extruder.h"
#include <Packet.h>
#include <stdint.h>

#define ABS_ZERO -273.15

#define NUM_TIMERS 2
#define DEBUG_TEMP_TIMER 0
#define TEMP_TIMER 1

extern Extruder extruders[];
extern uint8_t therms[], heaters[], enables[], steps[], dirs[];

extern unsigned long timers[];

// Global flag set when the buffer has a full command to be processed
extern uint8_t input_ready;

#endif // GLOBALS_H
