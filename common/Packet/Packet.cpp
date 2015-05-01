#include "Packet.h"

#include <stdint.h>
#include <util/crc16.h>
#include <string.h>

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

PacketQueue::PacketQueue() {
	_front = 0;
	_back = 0;
	time_ms = 0;
	wait_ms = 100;
}

const Packet * PacketQueue::front() {
	if( _front == _back )
		return 0;
	return &(queue[_front]);
}

uint8_t PacketQueue::enqueue( uint8_t command ) {
	enqueue( command, Payload() );
}

uint8_t PacketQueue::enqueue( uint8_t command, Payload payload ) {
	if( (_back+1) % PACKET_QUEUE_SIZE != _front ) {
		queue[_back].command = command;
		queue[_back].payload = payload;
		queue[_back].sign();
		_back = (_back+1) % PACKET_QUEUE_SIZE;
		return 1;
	} else {
		return 0;
	}
}

uint8_t PacketQueue::enqueue( Packet * entrant ) {
	if( (_back+1) % PACKET_QUEUE_SIZE != _front ) {
		memcpy( &(queue[_back]), entrant, sizeof( Packet ) );
		_back = (_back+1) % PACKET_QUEUE_SIZE;
		return 1;
	} else {
		return 0;
	}
}

void PacketQueue::dequeue() {
	if( _front == _back )
		return;
	_front = (_front + 1) % PACKET_QUEUE_SIZE;
	time_ms = 0;
	wait_ms = 100;
}

void Packet::print( HardwareSerial s ) const {
	char buf[64];
	sprintf( buf, "%02x | ", command );
	s.print( buf );
	for( uint8_t i = 0; i < sizeof( Payload ); i++ ) {
		sprintf( buf, "%02x ", payload.bytes[i] );
		s.print( buf );
	}
	sprintf( buf, "| %02x %02x", ((uint8_t*)(&crc))[0], ((uint8_t*)(&crc))[1] );
	s.print( buf );
}
