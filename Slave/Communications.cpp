#include "Communications.h"
#include "Globals.h"
#include <Packet.h>

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

void serialEvent1() {
	if( input_ready )
		return;

	while( (size_t) MASTER.available() >= sizeof( Packet ) ) {
		memset( &input_packet, 0, sizeof(Packet) );
		for( uint8_t i = 0; i < sizeof(Packet); i++ ) {
			((uint8_t*)(&input_packet))[i] = MASTER.read();
		}
		if( input_packet.check() ) {
			input_ready = 1;
			output_packet.command = CMD_ACK;
			output_packet.payload.crc = input_packet.crc;
			output_packet.sign();
			MASTER.write( (uint8_t*)(&output_packet), sizeof(Packet) );
			return;
		}
	}
}
