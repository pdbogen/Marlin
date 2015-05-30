#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include "Configuration.h"

#include <Packet.h>
#include <stdint.h>

//#define DEBUG_SERIAL_IO

#define SLAVE_STEP_PIN 16

#ifndef MASTER
#define MASTER SLAVE_SERIAL
#endif

#ifndef DEBUG_IO
#define DEBUG_IO MYSERIAL
#endif

void link_initialize();
void network_loop();

extern PacketQueue input_queue;
extern PacketQueue output_queue;

struct PacketHandler {
	uint8_t command;
	void (*handler)( const Payload & p );
	PacketHandler( uint8_t command, void(*handler)( const Payload & ) ) : command( command ), handler( handler ) { }
};

extern PacketHandler packetHandlers[];
void serialEvent1();

#endif
