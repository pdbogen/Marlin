#include "Communications.h"
#include <Packet.h>

#include <Arduino.h>
#include <stdint.h>
#include <stdlib.h>

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


uint8_t input_fragment[ sizeof( Packet ) ];
uint8_t fragment_index = 0;

void link_layer_synchronize() {
	// Not building a fragment, discard bytes until SYN
	if( fragment_index == 0 )
		while( MASTER.available() > 0 && MASTER.peek() != LINK_SYN ) MASTER.read();
}

uint8_t link_layer_read_bytes() {
	uint8_t c;
	uint8_t stop = 0;
	while( MASTER.available() > 0 && fragment_index < sizeof(Packet) && stop == 0 ) {
		switch( MASTER.peek() ) {
			// Last packet was an incomplete packet, discard and start over.
			case LINK_SYN:
				if( MASTER.available() >= 2 ) {
					MASTER.read();
					if( fragment_index > 0 ) {
						DEBUG_IO.print( "// Warning: dropping partial (" );
						DEBUG_IO.print( fragment_index );
						DEBUG_IO.println( " byte) packet" );
						fragment_index = 0;
						memset( input_fragment, 0, sizeof( Packet ) );
					}
				} else {
					stop = 1;
				}
				break;
			case LINK_ESC:
				if( MASTER.available() >= 2 ) {
					MASTER.read();
					switch( c = MASTER.read() ) {
						case ESC_SYN:
							input_fragment[ fragment_index++ ] = LINK_SYN;
							break;
						case ESC_ESC:
							input_fragment[ fragment_index++ ] = LINK_ESC;
							break;
						case LINK_SYN: // This isn't an escape sequence, it's a SYN!
							DEBUG_IO.print( "// Warning: dropping partial (" );
							DEBUG_IO.print( fragment_index );
							DEBUG_IO.println( " byte) packet" );
							fragment_index = 0;
							memset( input_fragment, 0, sizeof( Packet ) );
							break;
						default:
							input_fragment[ fragment_index++ ] = c;
							break;
					}
				} else {
					// Escape but no character, so fragment
					// ends in the middle of an escape
					// sequence.
					// This will leave the ESC on the buffer and we can try later.
					stop = 1;
				}
				break;
			default:
				input_fragment[ fragment_index++ ] = MASTER.read();
				break;
		}
	}
	return stop == 0;
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
	MASTER.flush();
}

void network_loop() {
	if( output_queue.front() && output_queue.time_ms + output_queue.wait_ms <= millis() ) {
		if( output_queue.wait_ms > 200 ) {
			DEBUG_IO.println( "// Retry limit exceeded, dropping output packet" );
			output_queue.dequeue();
		} else {
			link_transmit_packet( output_queue.front() );
			output_queue.time_ms = millis();
			output_queue.wait_ms += 10;
		}
	}
	const Packet * p;
	while( p = input_queue.front() ) {
		uint8_t handled = 0;
		for( PacketHandler * ph = packetHandlers; ph->command != 0; ph++ ) {
			if( ph->command == p->command ) {
				(*ph->handler)( p->payload );
				handled = 1;
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

void network_transmit_ack( uint16_t crc ) {
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
		link_layer_synchronize();

		// Return value of 0 means we needed to read a two-byte
		// something and couldn't, so let some time pass and come back.
		if( !link_layer_read_bytes() ) {
			return;
		}

		// Did we read a complete packet?
		if( fragment_index == sizeof( Packet ) ) {
			network_handle_packet( (Packet*) &(input_fragment) );
			fragment_index = 0;
			memset( input_fragment, 0, sizeof( Packet ) );
		}
	}
}

void link_initialize() {
	MASTER.begin(250000);
	// Flush the MASTER serial hardware buffer
	while( MASTER.available() )
		MASTER.read();

}
