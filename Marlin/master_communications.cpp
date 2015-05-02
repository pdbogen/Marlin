#include "master_communications.h"
#include "Communications.h"
#include "temperature.h"

void warn_bad_extruder( uint8_t extruder ) {
	DEBUG_IO.print( "// cannot send command to set temp for remote extruder #" );
	DEBUG_IO.println( extruder );
}

void store_extruder_temp_from_slave( const Payload &p ) {
	current_temperature[1] = p.decimal;
}

void slave_set_extruder_temperature( uint8_t extruder, float target ) {
	if( extruder == 0 ) {
		output_queue.enqueue( CMD_SET_TEMP_0, Payload( target ) );
	} else {
		warn_bad_extruder( extruder );
	}
}

void slave_set_extruder_enabled( uint8_t extruder ) {
	if( extruder == 0 ) {
		output_queue.enqueue( CMD_SET_ENABLE_0 );
	} else {
		warn_bad_extruder( extruder );
	}
}

void slave_set_extruder_disabled( uint8_t extruder ) {
	if( extruder == 0 ) {
		output_queue.enqueue( CMD_SET_DISABLE_0 );
	} else {
		warn_bad_extruder( extruder );
	}
}

void slave_send_step() {
	digitalWrite( SLAVE_STEP_PIN, !digitalRead( SLAVE_STEP_PIN ) );
}

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_REPORT_TEMP, &store_extruder_temp_from_slave ),
	PacketHandler( 0, NULL )
};

extern void slave_step_pin_initialize() {
	pinMode( SLAVE_STEP_PIN, OUTPUT );
}
