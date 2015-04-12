#include "Extruder.h"
#include "Globals.h"

#include <Arduino.h>
#include <math.h>

float Extruder::getTemperature() {
	float r = analogRead(temp_pin);
	temperature = ABS_ZERO + beta / log( (r*Rs/(AD_RANGE - r)) /RInf );
	return temperature;
}

void Extruder::calculateRInf() {
	RInf = NTC * exp( -beta / 298.15 );
}
