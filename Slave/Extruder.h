#ifndef EXTRUDER_H
#define EXTRUDER_H

#include "Configuration.h"

#include <Arduino.h>
#include <stdint.h>

struct Extruder {
	float target_temperature;
	float temperature;
	float beta;
	float integral_max;
	unsigned long Rs;
	float RInf;
	uint8_t temp_pin;

	Extruder() {
		temperature = 0;
		beta = 0;
		Rs = 0;
		RInf = 0;
		integral_max = 1000.0 * PID_INTERVAL;
		kp = 34; ki = 4.02; kd = 71;
		integral = 0; prev_error = 0; last=millis();
		temp_idx = 255;
	}
	float getTemperature( uint8_t smoothed = 1 );
	void calculateRInf();
	void setTemperature( float celsius ) { target_temperature = celsius; integral = 0; prev_error = 0; }
	void runPID();
	void setHeaterPin( uint8_t p ) { pinMode( p, OUTPUT ); heater_pin = p; }
	void setEnablePin( uint8_t p ) { pinMode( p, OUTPUT ); enable_pin = p; disable(); }
	void setStepPin( uint8_t p )   { pinMode( p, OUTPUT ); step_pin = p; digitalWrite( p, LOW ); }
	void enable() { digitalWrite( enable_pin, LOW ); }
	void disable() { digitalWrite( enable_pin, HIGH ); }
	void step() { digitalWrite( step_pin, !digitalRead( step_pin ) ); }
	uint8_t autotune( float temp, int ncycles );

private:
	uint16_t smooth( uint16_t );
	uint8_t kp, kd;
	float ki;
	float prev_error;
	float integral;
	unsigned long last;
	uint8_t heater_pin, enable_pin, step_pin, direction_pin;
	uint8_t temp_idx;
	uint16_t temps[TEMPERATURE_SAMPLES];
};

#endif // EXTRUDER_H
