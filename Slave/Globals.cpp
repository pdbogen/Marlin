#include "Configuration.h"
#include "Extruder.h"
#include "Globals.h"
#include "Pins.h"

Extruder extruders[HOT_ENDS];

uint8_t therms[] = THERMS;
unsigned long timers[NUM_TIMERS];
