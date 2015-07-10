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

void report_autotune_completion( const Payload &p ) {
/*	if( p.byte == 0 ) {
		DEBUG_IO.print( "// slave autotune finished successfully!" );
	} else {
		DEBUG_IO.print( "// slave autotune failed to complete! check slave serial for more information" );
	}
*/
}

void slave_autotune( uint8_t extruder, float temperature ) {
#if 0 // Disable slave autotune debug
	DEBUG_IO.print( "// instructing slave to autotune E" );
		DEBUG_IO.print( extruder );
		DEBUG_IO.print( " at S" );
		DEBUG_IO.println( temperature );
	for( uint8_t i = 0; i < sizeof( uint8_t ); i++ ) {
		DEBUG_IO.print( ((uint8_t*)(&extruder))[i], HEX );
		DEBUG_IO.print( " " );
	}
	DEBUG_IO.println();
	for( uint8_t i = 0; i < sizeof( float ); i++ ) {
		DEBUG_IO.print( ((uint8_t*)(&temperature))[i], HEX );
		DEBUG_IO.print( " " );
	}
	DEBUG_IO.println();
#endif // Disable slave autotune debug
	output_queue.enqueue( CMD_AUTOTUNE, Payload( extruder, temperature ) );
}

void slave_set_extruder_temperature( uint8_t extruder, float target ) {
	if( extruder == 0 ) {
		output_queue.enqueue( CMD_SET_TEMP_0, Payload( target ) );
	} else {
		warn_bad_extruder( extruder );
	}
}

void slave_set_extruder_enable( uint8_t extruder, uint8_t state ) {
	if( extruder == 0 ) {
		static uint8_t last_0 = !state;
		if( state != last_0 ) {
			last_0 = state;
			if( state == 1 ) {
				output_queue.enqueue( CMD_SET_ENABLE_0 );
			} else {
				output_queue.enqueue( CMD_SET_DISABLE_0 );
			}
		}
	}
}

void slave_set_extruder_direction( uint8_t extruder, uint8_t direction ) {
	if( extruder == 0 ) {
		static uint8_t last = !direction;
		if( direction != last ) {
			output_queue.enqueue( CMD_SET_DIRECTION_0, Payload( direction ) );
			last = direction;
		}
	} else {
		warn_bad_extruder( extruder );
	}
}

void slave_send_step( uint8_t state ) {
	WRITE( SLAVE_STEP_PIN, state );
}

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_REPORT_TEMP, &store_extruder_temp_from_slave ),
	PacketHandler( CMD_AUTOTUNE_DONE, &report_autotune_completion ),
	PacketHandler( 0, NULL )
};

extern void slave_step_pin_initialize() {
	pinMode( SLAVE_STEP_PIN, OUTPUT );
}
