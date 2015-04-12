#ifndef GLOBALS_H
#define GLOBALS_H

#include "Configuration.h"
#include "Extruder.h"
#include <Packet.h>
#include <stdint.h>

#define ABS_ZERO -273.15

#define NUM_TIMERS 1
#define DEBUG_TEMP_TIMER 0

extern Extruder extruders[];
extern uint8_t therms[];

extern unsigned long timers[];

// Global flag set when the buffer has a full command to be processed
extern uint8_t input_ready;

extern Packet input_packet;
extern Packet output_packet;

#endif // GLOBALS_H
