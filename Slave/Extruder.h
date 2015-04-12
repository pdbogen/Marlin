#ifndef EXTRUDER_H
#define EXTRUDER_H

#include <stdint.h>

struct Extruder {
	float temperature;
	float beta;
	unsigned long Rs;
	float RInf;
	uint8_t temp_pin;

	Extruder() {
		temperature = 0;
		beta = 0;
		Rs = 0;
		RInf = 0;
	}
	float getTemperature();
	void calculateRInf();
};

#endif // EXTRUDER_H
