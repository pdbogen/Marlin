#include "Communications.h"
#include "Globals.h"
#include "Temperature.h"

void report_ping( const Payload & ) { DEBUG_IO.println( "Master said hello!" ); }

void set_temp_0( const Payload & p ) {
	DEBUG_IO.print( "Setting extruder 0 temp to " );
	DEBUG_IO.println( p.decimal );
	set_hotend_temperature( 0, p.decimal );
}

void set_enable_0( const Payload & )  { DEBUG_IO.println( " Enabling Extruder 0" ); extruders[0].enable();  }
void set_disable_0( const Payload & ) { DEBUG_IO.println( "Disabling Extruder 0" ); extruders[0].disable(); }
void cmd_autotune( const Payload & p ) {
	if( p.byte < 0 || p.byte > HOT_ENDS ) {
		DEBUG_IO.print( "Cannot autotune non-existent hot end " + p.byte );
		return;
	}
	output_queue.enqueue( CMD_AUTOTUNE_DONE, extruders[ p.byte ].autotune( 185, 10 ) );
}

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_PING, &report_ping ),
	PacketHandler( CMD_SET_TEMP_0,   &set_temp_0     ),
	PacketHandler( CMD_SET_ENABLE_0, &set_enable_0   ),
	PacketHandler( CMD_SET_DISABLE_0, &set_disable_0 ),
	PacketHandler( CMD_AUTOTUNE,        &cmd_autotune    ),
	PacketHandler( 0, NULL )
};

void slave_step_pin_initialize() {
	pinMode( SLAVE_STEP_PIN, INPUT );
}
