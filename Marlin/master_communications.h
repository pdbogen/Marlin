#ifndef _MASTER_COMMUNICATIONS
#define _MASTER_COMMUNICATIONS

#include <HardwareSerial.h>
#include "Communications.h"
#include "Configuration.h"
#include <stdint.h>

void warn_bad_extruder( uint8_t extruder );
void slave_set_extruder_temperature( uint8_t extruder, float target );
void slave_set_extruder_enable( uint8_t extruder, uint8_t state );
void slave_autotune( uint8_t extruder, float temperature );

extern uint8_t remote_extruder_direction[];

inline void slave_set_extruder_direction( uint8_t extruder, uint8_t direction ) {
	if( extruder == 0 ) {
		if( direction != remote_extruder_direction[0] ) {
			output_queue.enqueue( CMD_SET_DIRECTION_0, Payload( direction ) );
			remote_extruder_direction[0] = direction;
		}
	} else {
		warn_bad_extruder( extruder );
	}
}

#endif
