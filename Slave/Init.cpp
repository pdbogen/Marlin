#include "Init.h"
#include "Configuration.h"
#include "Globals.h"
#include "Pins.h"
#include "Communications.h"

#include <Arduino.h>
#include <HardwareSerial.h>
#include <stdint.h>

void initialize_serial() {
	DEBUG_IO.begin(DEBUG_BAUD);
	link_initialize();
}

void initialize_temperatures() {
	unsigned long betas[] = BETAS;
	unsigned long rs[]    = RS;

	for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
		pinMode( therms[i], INPUT );
		extruders[i].temp_pin   = therms[i];
		extruders[i].setEnablePin( enables[i] );
		extruders[i].setHeaterPin( heaters[i] );
		extruders[i].setStepPin(   steps[i]   );
		extruders[i].setDirectionPin( dirs[i] );
		extruders[i].beta     = betas[i];
		extruders[i].Rs       = rs[i];
		extruders[i].calculateRInf();
	}
}
