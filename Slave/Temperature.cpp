#include "Temperature.h"
#include "Configuration.h"
#include "Globals.h"

#include <stdint.h>
#include <Arduino.h>

void check_hotend_temperatures() {
	for( uint8_t i = 0; i < HOT_ENDS; i++ )
		extruders[i].getTemperature();
	unsigned long time = millis();

	if( (time - timers[DEBUG_TEMP_TIMER]) >= 1000 ) {
		timers[DEBUG_TEMP_TIMER] = time;
		for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
			Serial.print( "Extruder " ); Serial.print( i ); Serial.print( " temp: " ); Serial.println( extruders[i].temperature );
		}
	}
}
