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

void Extruder::runPID() {
	unsigned long dt = millis() - last;
	if( dt < PID_INTERVAL )
		return;

	float error = target_temperature - temperature;
	if( error > 10 || error < -10 ) {
		integral = 0;
	} else {
		integral = integral + error * dt;
		if( integral > integral_max )
			integral = integral_max;
		if( integral < -integral_max )
			integral = -integral_max;
	}
	float derivative = (error - prev_error) / dt;
	prev_error = error;
	float output = error * kp + integral * ki + derivative * kd;
	#ifdef DEBUG_PID
		DEBUG_IO.print( "measurement=" ); DEBUG_IO.print( temperature ); DEBUG_IO.print( " setpoint=" ); DEBUG_IO.println( target_temperature );
		DEBUG_IO.print( "P=" ); DEBUG_IO.print( error ); DEBUG_IO.print( " I=" ); DEBUG_IO.print( integral ); DEBUG_IO.print( "D=" ); DEBUG_IO.println( derivative );
		DEBUG_IO.print( "output=" ); DEBUG_IO.print( output );
	#endif
	if( output < 0 ) {
		digitalWrite( heater_pin, LOW );
		#ifdef DEBUG_PID
			DEBUG_IO.println( " set=LOW" );
		#endif
	} else if( output >= 255 ) {
		digitalWrite( heater_pin, HIGH );
		#ifdef DEBUG_PID
			DEBUG_IO.println( " set=HIGH" );
		#endif
	} else {
		analogWrite( heater_pin, output );
		#ifdef DEBUG_PID
			DEBUG_IO.print( " set=" ); DEBUG_IO.println( (uint8_t) output );
		#endif
	}
	last = dt + last;
}
