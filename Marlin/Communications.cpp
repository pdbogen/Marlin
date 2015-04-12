#include "Communications.h"

void setup_slave_communications() {
	SLAVE_SERIAL.begin( SLAVE_BAUDRATE );
}

