#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "Communications.h"

#include <Arduino.h>

ISR( PCINT2_vect ) {
	for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
		extruders[i].step();
	}
}

void initialize_interrupts() {
	PCICR  |= _BV(PCIE2);
	PCMSK2 |= _BV(PCINT16);
	MCUCR   = _BV(ISC01) | _BV(ISC00); // Rising and falling edge trigger
}

#endif
