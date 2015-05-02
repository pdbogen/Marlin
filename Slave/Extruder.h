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
		kp = 20; ki = 0.3 / PID_INTERVAL; kd = 10;
		integral = 0; prev_error = 0; last=millis();
	}
	float getTemperature();
	void calculateRInf();
	void setTemperature( float celsius ) { target_temperature = celsius; integral = 0; prev_error = 0; }
	void runPID();
	void setHeaterPin( uint8_t p ) { pinMode( p, OUTPUT ); heater_pin = p; }
	void setEnablePin( uint8_t p ) { pinMode( p, OUTPUT ); enable_pin = p; disable(); }
	void setStepPin( uint8_t p )   { pinMode( p, OUTPUT ); step_pin = p; digitalWrite( p, LOW ); }
	void enable() { digitalWrite( enable_pin, LOW ); }
	void disable() { digitalWrite( enable_pin, HIGH ); }
	void step() { digitalWrite( step_pin, !digitalRead( step_pin ) ); }

private:
	uint8_t kp, kd;
	float ki;
	float prev_error;
	float integral;
	unsigned long last;
	uint8_t heater_pin, enable_pin, step_pin;
};

#endif // EXTRUDER_H
