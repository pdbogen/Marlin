#include "Packet.h"

#include <stdint.h>
#include <util/crc16.h>

void Packet::sign() {
	crc = 0;
	crc = _crc16_update( crc, command );
	for( uint8_t i = 0; i < (sizeof(payload)); i++ )
		crc = _crc16_update( crc, payload.bytes[i] );
		//((uint8_t*)(payload))[i] );
}

uint8_t Packet::check() {
	uint16_t crc_calc = 0;
	crc_calc = _crc16_update( crc_calc, command );
	for( uint8_t i = 0; i < (sizeof(payload)); i++ )
		crc_calc = _crc16_update( crc_calc, payload.bytes[i] );
		//((uint8_t*)(payload))[i] );
	return( crc_calc == crc );
}
