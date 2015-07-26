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
	uint8_t integral_range;

	Extruder() {
		temperature = 0;
		beta = 0;
		Rs = 0;
		RInf = 0;
		integral_range = 15;
//		kp = 20; ki = .0001; kd = 0;
		kp = 8; ki=.0007; kd=0;
		integral = 0; prev_error = 0; last=millis();
		temp_idx = 255;
	}
	void setTargetTemperature( float celsius ) { target_temperature = celsius; }
	void calculateRInf();
	void runPID();
	void setHeaterPin( uint8_t p ) { pinMode( p, OUTPUT ); heater_pin = p; }
	void setEnablePin( uint8_t p ) { pinMode( p, OUTPUT ); enable_pin = p; disable(); }
	void setStepPin( uint8_t p )   { pinMode( p, OUTPUT ); step_pin = p; digitalWrite( p, LOW ); }
	void setDirectionPin( uint8_t p ) { pinMode( p, OUTPUT ); direction_pin = p; }
	void enable() { digitalWrite( enable_pin, LOW ); }
	void disable() { digitalWrite( enable_pin, HIGH ); }
	void step() { digitalWrite( step_pin, !digitalRead( step_pin ) ); }
	void direction( uint8_t dir ) { digitalWrite( direction_pin, dir ); }
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
//	uint16_t temps[TEMPERATURE_SAMPLES];
	uint16_t prev_temp;
};

#endif // EXTRUDER_H
