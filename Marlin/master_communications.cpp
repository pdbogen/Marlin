#include "master_communications.h"
#include "Communications.h"
#include "temperature.h"

void store_extruder_temp_from_slave( const Payload &p ) {
	current_temperature[1] = p.decimal;
}

void slave_set_extruder_temperature( uint8_t extruder, float target ) {
	if( extruder == 0 ) {
		output_queue.enqueue( SET_TEMP_0, Payload( target ) );
	} else {
		DEBUG_IO.print( "// cannot send command to set temp for remote extruder #" );
		DEBUG_IO.println( extruder );
	}
}

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_REPORT_TEMP, &store_extruder_temp_from_slave ),
	PacketHandler( 0, NULL )
};

