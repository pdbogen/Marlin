#ifndef _TEMPERATURE_H
#define _TEMPERATURE_H

#include <stdint.h>

void check_hotend_temperatures();
void set_hotend_temperature( uint8_t extruder, float celsius );
void run_hotend_pid();

#endif // _TEMPERATURE_H
