#include "Communications.h"
#include "Temperature.h"

void report_ping( const Payload & ) { DEBUG_IO.println( "Master said hello!" ); }

void set_temp_0( const Payload & p ) { set_hotend_temperature( 0, p.decimal ); }

PacketHandler packetHandlers[] = {
	PacketHandler( CMD_PING, &report_ping ),
	PacketHandler( CMD_SET_TEMP_0, &set_temp_0 ),
	PacketHandler( 0, NULL )
};
