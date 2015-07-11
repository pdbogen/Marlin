#include "Temperature.h"
#include "Communications.h"
#include "Configuration.h"
#include "Globals.h"
#include "ThermistorTables.h"

#include <stdint.h>
#include <Arduino.h>

float analog2temp(unsigned long raw, uint8_t e);

enum TempState {
	PrepareTemp,
	MeasureTemp,
	StartupDelay // Startup, delay initial temp reading a tiny bit so the hardware can settle
};

void run_hotend_pid() {
	for( uint8_t i = 0; i < HOT_ENDS; i++ )
		extruders[i].runPID();
}

void set_hotend_temperature( uint8_t extruder, float celsius ) {
	extruders[ extruder ].setTargetTemperature( celsius );
}

// Writing ADSC to 1 in ADCSRA starts a conversion
#define SET_ADMUX_ADCSRA(pin) ADMUX = _BV(REFS0) | (pin & 0x07); ADCSRA |= _BV(ADSC)
#define START_ADC(pin) ADCSRB = 0; SET_ADMUX_ADCSRA(pin)

void check_hotend_temperatures() {
	static TempState temp_state = StartupDelay;
	static uint8_t temp_count = 0;
	static uint8_t extruder = 0;
	static unsigned long raw_temp_value[HOT_ENDS] = { 0 };

	switch(temp_state) {
		case PrepareTemp:
			START_ADC(extruders[extruder].temp_pin);
			temp_state = MeasureTemp;
			break;
		case MeasureTemp:
			// ADIF is set once a measurement is complete
			if( ADCSRA & _BV(ADIF) ) {
				raw_temp_value[extruder] += ADC;
				temp_state = PrepareTemp;
				extruder++;
				if( extruder >= HOT_ENDS ) {
					extruder = 0;
					temp_count++;
				}
			}
			break;
		case StartupDelay:
			temp_state = PrepareTemp;
			break;
	}

	if( temp_count >= OVERSAMPLENR ) {
		temp_count = 0;
		for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
			extruders[i].temperature = analog2temp( raw_temp_value[i], i );
			raw_temp_value[i] = 0;
		}
	}

	unsigned long time = millis();
	if( ( time - timers[DEBUG_TEMP_TIMER]) >= 100 ) {
		timers[DEBUG_TEMP_TIMER] = time;
		for( uint8_t i = 0; i < HOT_ENDS; i++ ) {
			output_queue.enqueue( CMD_REPORT_TEMP, Payload( extruders[i].temperature ) );
		}
	}
}

#if HOT_ENDS > 3
	#define ARRAY_BY_EXTRUDERS(v1, v2, v3, v4) { v1, v2, v3, v4 }
#elif HOT_ENDS > 2
	#define ARRAY_BY_EXTRUDERS(v1, v2, v3, v4) { v1, v2, v3 }
#elif HOT_ENDS > 1
	#define ARRAY_BY_EXTRUDERS(v1, v2, v3, v4) { v1, v2 }
#else
	#define ARRAY_BY_EXTRUDERS(v1, v2, v3, v4) { v1 }
#endif
#define PGM_RD_W(x)   (short)pgm_read_word(&x)

// Derived from RepRap FiveD extruder::getTemperature()
// For hot end temperature measurement.
float analog2temp(unsigned long raw, uint8_t e) {
	static void *  heater_ttbl_map[HOT_ENDS] = ARRAY_BY_EXTRUDERS( (void *)HEATER_0_TEMPTABLE, (void *)HEATER_1_TEMPTABLE, (void *)HEATER_2_TEMPTABLE, (void *)HEATER_3_TEMPTABLE );
	static uint8_t heater_ttbllen_map[HOT_ENDS] = ARRAY_BY_EXTRUDERS( HEATER_0_TEMPTABLE_LEN, HEATER_1_TEMPTABLE_LEN, HEATER_2_TEMPTABLE_LEN, HEATER_3_TEMPTABLE_LEN );

	if (e >= HOT_ENDS) {
		return 0.0;
	}

	if (heater_ttbl_map[e] != NULL) {
		float celsius = 0;
		uint8_t i;
		short (*tt)[][2] = (short (*)[][2])(heater_ttbl_map[e]);

		for (i = 1; i < heater_ttbllen_map[e]; i++) {
			if (PGM_RD_W((*tt)[i][0]) > raw) {
				celsius = PGM_RD_W((*tt)[i-1][1]) +
				          (raw - PGM_RD_W((*tt)[i-1][0])) *
				          (float)(PGM_RD_W((*tt)[i][1]) - PGM_RD_W((*tt)[i-1][1])) /
				          (float)(PGM_RD_W((*tt)[i][0]) - PGM_RD_W((*tt)[i-1][0]));
				break;
			}
		}
		// Overflow: Set to last value in the table
		if (i == heater_ttbllen_map[e]) celsius = PGM_RD_W((*tt)[i-1][1]);
		return celsius;
	}
	return ((raw * ((5.0 * 100.0) / 1024.0) / OVERSAMPLENR) * TEMP_SENSOR_AD595_GAIN) + TEMP_SENSOR_AD595_OFFSET;
}
