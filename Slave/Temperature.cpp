#include "Temperature.h"
#include "Communications.h"
#include "Configuration.h"
#include "Globals.h"

#include <stdint.h>
#include <Arduino.h>

void run_hotend_pid() {
	for( uint8_t i = 0; i < HOT_ENDS; i++ )
		extruders[i].runPID();
}

void set_hotend_temperature( uint8_t extruder, float celsius ) {
	extruders[ extruder ].setTemperature( celsius );
}

void check_hotend_temperatures() {
	unsigned long time = millis();
	if( (time - timers[TEMP_TIMER]) >= 10 ) {
		timers[TEMP_TIMER] = time;
		for( uint8_t i = 0; i < HOT_ENDS; i++ )
			extruders[i].getTemperature();
	}

	if( (time - timers[DEBUG_TEMP_TIMER]) >= 1000 ) {
		timers[DEBUG_TEMP_TIMER] = time;
		for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
			output_queue.enqueue( CMD_REPORT_TEMP, Payload( extruders[i].temperature ) );
		}
	}
}
