#ifndef _MASTER_COMMUNICATIONS
#define _MASTER_COMMUNICATIONS

#include <stdint.h>

void slave_set_extruder_temperature( uint8_t extruder, float target );
void slave_set_extruder_enabled( uint8_t extruder );
void slave_set_extruder_disabled( uint8_t extruder );
void slave_send_step();

#endif
