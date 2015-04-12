#ifndef PACKET_H
#define PACKET_H

#define CMD_ACK 0

#include <stdint.h>

struct Packet {
	uint8_t command;
	union {
		long integer;
		unsigned long uinteger;
		float decimal;
		uint16_t crc;
		uint8_t  bytes[];
	} payload;
	uint16_t crc;
	void sign();
	uint8_t check();
};

#endif // PACKET_H
