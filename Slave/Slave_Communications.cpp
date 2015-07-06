#include "Communications.h"
#include "Globals.h"
#include "Temperature.h"

void report_ping( const Payload & ) { DEBUG_IO.println( "Master said hello!" ); }

void set_temp_0( const Payload & p ) {
#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.print( "Setting extruder 0 temp to " );
	DEBUG_IO.println( p.decimal );
#endif // DEBUG_SERIAL_IO
	set_hotend_temperature( 0, p.decimal );
}

void set_enable_0( const Payload & )  {
#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.println( " Enabling Extruder 0" );
#endif // DEBUG_SERIAL_IO
	extruders[0].enable();
}
void set_disable_0( const Payload & ) {
#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.println( "Disabling Extruder 0" );
#endif // DEBUG_SERIAL_IO
	extruders[0].disable();
}
void set_direction_0( const Payload & p ) {
#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.print( "Setting Extruder 0 Direction to " );
	DEBUG_IO.println( p.byte );
#endif // DEBUG_SERIAL_IO
	extruders[0].direction( p.byte );
}

void cmd_autotune( const Payload & p ) {
#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.print( "Received command to autotune: " );
	for( uint8_t i = 0; i < sizeof( Payload ); i++ ) {
		DEBUG_IO.print( p.bytes[i], HEX );
		DEBUG_IO.print( " " );
	}
	DEBUG_IO.println();
	for( uint8_t i = 0; i < sizeof( extruder_temp ); i++ ) {
		DEBUG_IO.print( ((uint8_t*)&(p.etemp))[i], HEX );
		DEBUG_IO.print( " " );
	}
	DEBUG_IO.println();
	DEBUG_IO.print( p.etemp.extruder );
		DEBUG_IO.print( " " );
		DEBUG_IO.println( p.etemp.temperature );
#endif // DEBUG_SERIAL_IO
	if( p.etemp.extruder < 0 || p.etemp.extruder > HOT_ENDS ) {
#ifdef DEBUG_SERIAL_IO
		DEBUG_IO.print( "Cannot autotune non-existent hot end " + p.etemp.extruder );
#endif // DEBUG_SERIAL_IO
		return;
	}
	output_queue.enqueue( CMD_AUTOTUNE_DONE, extruders[ p.etemp.extruder ].autotune( p.etemp.temperature, 8 ) );
}

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_PING, &report_ping ),
	PacketHandler( CMD_SET_TEMP_0,      &set_temp_0      ),
	PacketHandler( CMD_SET_ENABLE_0,    &set_enable_0    ),
	PacketHandler( CMD_SET_DISABLE_0,   &set_disable_0   ),
	PacketHandler( CMD_SET_DIRECTION_0, &set_direction_0 ),
	PacketHandler( CMD_AUTOTUNE,        &cmd_autotune    ),
	PacketHandler( 0, NULL )
};

void slave_step_pin_initialize() {
	pinMode( SLAVE_STEP_PIN, INPUT );
}
