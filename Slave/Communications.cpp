#include "Communications.h"

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef MASTER_BAUD
#define MASTER_BAUD SLAVE_BAUDRATE
#endif

#define LINK_SYN 0x01
#define LINK_ESC 0x10
#define ESC_SYN 0x53
#define ESC_ESC 0x45

PacketQueue input_queue;
PacketQueue output_queue;

// Expected call chain:
// Receive:
// serialEvent1
// + link_layer_synchronize
// + link_layer_read_bytes
// + network_handle_packet
// | + Packet::check
// | + output_queue.front (if ACK)
// | + output_queue.dequeue (if ACK matches)
// | + input_queue.enqueue (if not ACK)
// | + network_transmit_ack (if not ACK)
// | | + link_transmit_packet

// Transmit:
// (in loop)
// + network_loop
// | + link_transmit_packet( output_queue.front ) // if millis() > (time_ms+wait_ms)


extern void slave_step_pin_initialize();

// The packet size on the wire might be bigger due to escapes,
// but we unescape before adding bytes to input_fragment.
uint8_t input_fragment[ sizeof( Packet ) ];
uint8_t fragment_index = 0;

// Returns: 0 if a packet cannot be read, 1 if it may (no guarantee)
inline uint8_t link_layer_synchronize() {
	// Not building a fragment, discard bytes until SYN
	if( fragment_index == 0 )
		while( MASTER.available() > 0 && MASTER.peek() != LINK_SYN ) MASTER.read();
	if( MASTER.available() >= sizeof( Packet ) + 1 )
		return 1;
	else
		return 0;
}

// Returns: 0 if a packet could not be read, 1 if it could
uint8_t link_layer_read_bytes() {
	uint8_t c;
	while( MASTER.available() > 0 && fragment_index < sizeof(Packet) ) {
		switch( MASTER.peek() ) {
			// The first byte should be a SYN, but don't read it off
			// unless there's more bytes. If there aren't more bytes, we'll
			// come back to this later.
			case LINK_SYN:
				if( MASTER.available() >= 2 ) {
					MASTER.read();
					// Last packet was an incomplete packet, discard and start over.
					if( fragment_index > 0 ) {
						fragment_index = 0;
						if( link_layer_synchronize() == 0 )
							return 0;
					}
				} else {
					return 0;
				}
				break;
			case LINK_ESC:
				if( MASTER.available() >= 2 ) {
					MASTER.read();
					switch( c = MASTER.read() ) {
						case ESC_SYN:
							if( fragment_index >= sizeof(Packet) )
								DEBUG_IO.println( "// Error: Out of bound write __FILE__:__LINE__" );
							input_fragment[ fragment_index++ ] = LINK_SYN;
							break;
						case ESC_ESC:
							if( fragment_index >= sizeof(Packet) )
								DEBUG_IO.println( "// Error: Out of bound write __FILE__:__LINE__" );
							input_fragment[ fragment_index++ ] = LINK_ESC;
							break;
						case LINK_SYN: // This isn't an escape sequence, it's a SYN!
							fragment_index = 0;
							if( link_layer_synchronize() == 0 )
								return 0;
							break;
						default: // ESC+anything else is just that thing
							if( fragment_index >= sizeof(Packet) )
								DEBUG_IO.println( "// Error: Out of bound write __FILE__:__LINE__" );
							input_fragment[ fragment_index++ ] = c;
							break;
					}
				} else {
					// Escape but no character, so fragment
					// ends in the middle of an escape
					// sequence.
					// This will leave the ESC on the buffer and we can try later.
					return 0;
				}
				break;
			default:
				if( fragment_index >= sizeof(Packet) )
					DEBUG_IO.println( "// Error: Out of bound write __FILE__:__LINE__" );
				input_fragment[ fragment_index++ ] = MASTER.read();
				break;
		}
	}
	return 1;
}

void link_transmit_packet( const Packet * p ) {
	MASTER.write( LINK_SYN );
	for( uint8_t i = 0; i < sizeof( Packet ); i++ ) {
		switch( ((uint8_t*)(p))[i] ) {
			case LINK_SYN:
				MASTER.write( LINK_ESC );
				MASTER.write( ESC_SYN );
				break;
			case LINK_ESC:
				MASTER.write( LINK_ESC );
				MASTER.write( ESC_ESC );
				break;
			default:
				MASTER.write( ((uint8_t*)(p))[i] );
				break;
		}
	}
}

void network_loop() {
	if( output_queue.front() && (output_queue.time_ms + output_queue.wait_ms) <= millis() ) {
		if( output_queue.wait_ms > 200 ) {
			DEBUG_IO.print( "// Retry limit exceeded, dropping output packet " );
			output_queue.front()->print( DEBUG_IO );
			DEBUG_IO.println();
			output_queue.dequeue();
		} else {
			link_transmit_packet( output_queue.front() );
			output_queue.time_ms = millis();
			output_queue.wait_ms += 10;
		}
	}
	const Packet * p;
	while( (p = input_queue.front()) ) {
		uint8_t handled = 0;
		for( PacketHandler * ph = packetHandlers; ph->command != 0; ph++ ) {
			if( ph->command == p->command ) {
				(*ph->handler)( p->payload );
				handled = 1;
				break;
			}
		}
		if( !handled ) {
			DEBUG_IO.print( "// FIXME: No handler registered for packet " );
			p->print( DEBUG_IO );
			DEBUG_IO.println();
		}
		input_queue.dequeue();
	}
}

inline void network_transmit_ack( uint16_t crc ) {
	Packet p;
	p.command = CMD_ACK;
	p.payload.crc = crc;
	p.sign();
	#ifdef DEBUG_SERIAL_IO
	DEBUG_IO.print( "// Transmitting ACK: " );
	p.print( DEBUG_IO );
	DEBUG_IO.println();
	#endif
	link_transmit_packet( &p );
}

void network_handle_packet( Packet * p ) {
	// Drop mangled packets.
	if( !p->check() ) {
		#ifdef DEBUG_SERIAL_IO
			DEBUG_IO.print( "// Dropping malformed packet: " );
			p->print( DEBUG_IO );
			DEBUG_IO.println();
		#endif
		return;
	}

	// ACK packets are handled here; if the ACK is valid and is for the front of output_queue
	// then we know output_queue was received successfully, so dequeue it.
	if( p->command == CMD_ACK ) {
		if( output_queue.front() && output_queue.front()->crc == p->payload.crc ) {
			output_queue.dequeue();
		#ifdef DEBUG_SERIAL_IO
		} else {
			DEBUG_IO.print( "// Received unexpected ACK:" );
			p->print( DEBUG_IO );
			DEBUG_IO.println();
			DEBUG_IO.print( "//                    head:" );
			output_queue.front()->print( DEBUG_IO );
			DEBUG_IO.println();
		#endif
		}
		return;
	}

	// Add the packet to the queue or drop it if the queue is full
	if( input_queue.enqueue( p ) ) {
		#ifdef DEBUG_SERIAL_IO
			DEBUG_IO.print( "// Received and enqueued new packet: " );
			p->print( DEBUG_IO );
			DEBUG_IO.println();
		#endif
		network_transmit_ack( p->crc );
	} else {
		DEBUG_IO.println( "// Warning: input queue is full, could not enqueue packet" );
	}
}

// This is called after loop() by Arduino's main(). There's no magic here, it's
// just a convenience

// It only fires when there's data ready, i.e., when Serial1.available() returns
// true.
void serialEvent1() {
	while( MASTER.available() ) {
		// If fragment_index == 0, discard bytes until SYN
		if( link_layer_synchronize() == 0 )
			return;

		// Return value of 0 means we needed to read a two-byte
		// something and couldn't, so let some time pass and come back.
		if( link_layer_read_bytes() == 0 )
			return;

		// Did we read a complete packet?
		if( fragment_index == sizeof( Packet ) ) {
			network_handle_packet( (Packet*) &(input_fragment) );
			fragment_index = 0;
			return; // this means we process at most one packet per call to serialEvent1
		}
	}
}

void link_initialize() {
	slave_step_pin_initialize();
	MASTER.begin(MASTER_BAUD);
	// Flush the MASTER serial hardware buffer
	while( MASTER.available() )
		MASTER.read();
}
