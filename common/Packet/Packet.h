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
#include <string.h>

#ifndef PACKET_TEST
#include <HardwareSerial.h>
#endif // PACKET_TEST

struct extruder_temp {
	uint8_t extruder;
	float   temperature;
};

union Payload {
	long integer;
	unsigned long uinteger;
	float decimal;
	uint16_t crc;
	uint8_t byte;
	const uint8_t bytes[];
	uint8_t _bytes[];
	extruder_temp etemp;
	Payload() { memset( _bytes, 0, sizeof( Payload ) ); }
	Payload( uint8_t byte ) : byte( byte ) { }
	Payload( long integer ) : integer( integer ) { }
	Payload( unsigned long uinteger ) : uinteger( uinteger ) { }
	Payload( float decimal ) : decimal( decimal ) { }
	Payload( uint16_t crc ) : crc( crc ) { }
	Payload( const Payload &p ) { memcpy( _bytes, p._bytes, sizeof( Payload ) ); }
	Payload( const uint8_t extruder, const float temperature ) {
		etemp.extruder = extruder;
		etemp.temperature = temperature;
	}
	Payload& operator=(const Payload &p) { memcpy( _bytes, p._bytes, sizeof( Payload ) ); return *this; }
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
#ifndef PACKET_TEST
	void print( HardwareSerial s ) const;
#else // PACKET_TEST
	void print() const;
#endif // PACKET_TEST
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
