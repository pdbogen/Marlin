#include "Init.h"
#include "Configuration.h"
#include "Globals.h"
#include "Pins.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdint.h>

void initialize_serial() {
	DEBUG_IO.begin(250000);
	MASTER.begin(250000);
}

void initialize_temperatures() {
	unsigned long betas[] = BETAS;
	unsigned long rs[]    = RS;

	for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
		pinMode( therms[i], INPUT );
		extruders[i].temp_pin = therms[i];
		extruders[i].beta     = betas[i];
		extruders[i].Rs       = rs[i];
		extruders[i].calculateRInf();
	}
}
