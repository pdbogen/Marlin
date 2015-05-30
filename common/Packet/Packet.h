#ifndef PACKET_H
#define PACKET_H

#ifndef PACKET_QUEUE_SIZE
#define PACKET_QUEUE_SIZE 10
#endif

#define CMD_ACK  0
#define CMD_PING 1
#define CMD_REPORT_TEMP 2

#define CMD_SET_TEMP_0      3
#define CMD_SET_ENABLE_0    4
#define CMD_SET_DISABLE_0   5
#define CMD_SET_DIRECTION_0 6

#define CMD_AUTOTUNE 7
#define CMD_AUTOTUNE_DONE 8

#include <stdint.h>

#include <HardwareSerial.h>

union Payload {
	long integer;
	unsigned long uinteger;
	float decimal;
	uint16_t crc;
	uint8_t byte;
	const uint8_t bytes[];
	Payload() {}
	Payload( uint8_t byte ) : byte( byte ) { }
	Payload( long integer ) : integer( integer ) { }
	Payload( unsigned long uinteger ) : uinteger( uinteger ) { }
	Payload( float decimal ) : decimal( decimal ) { }
	Payload( uint16_t crc ) : crc( crc ) { }
	Payload( const Payload &p ) : uinteger( p.uinteger ) { }
	Payload& operator=(const Payload &p) { uinteger = p.uinteger; return *this; }
};

struct Packet {
	uint8_t command;
	Payload payload;
	uint16_t crc;
	void sign();
	uint8_t check();
	Packet() {}
	Packet( uint8_t command ) : command( command ) { sign(); }
	Packet( uint8_t command, Payload payload ) : command( command ), payload( payload ) { sign(); }
	void print( HardwareSerial s ) const;
};

// Simple ring-buffer-based queue
class PacketQueue {
private:
	Packet queue[PACKET_QUEUE_SIZE];
	uint8_t _front, _back;
public:
	PacketQueue();
	const Packet * front();
	void dequeue();
	uint8_t enqueue( Packet * );
	uint8_t enqueue( uint8_t command );
	uint8_t enqueue( uint8_t command, Payload payload );
	unsigned long time_ms;
	uint16_t wait_ms;
};

#endif // PACKET_H
