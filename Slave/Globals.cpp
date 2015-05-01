#include "Configuration.h"
#include "Extruder.h"
#include "Globals.h"
#include "Pins.h"

Extruder extruders[HOT_ENDS];

uint8_t therms[] = THERMS;
uint8_t heaters[] = HEATERS;

unsigned long timers[NUM_TIMERS];

uint8_t input_ready = 0;

Packet input_packet, output_packet;
