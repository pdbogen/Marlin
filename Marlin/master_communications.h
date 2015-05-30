#ifndef _MASTER_COMMUNICATIONS
#define _MASTER_COMMUNICATIONS

#include <stdint.h>

void slave_set_extruder_temperature( uint8_t extruder, float target );
void slave_set_extruder_enable( uint8_t extruder, uint8_t state );
void slave_set_extruder_direction( uint8_t extruder, uint8_t direction );
void slave_send_step();
void slave_autotune( uint8_t extruder );

#endif
