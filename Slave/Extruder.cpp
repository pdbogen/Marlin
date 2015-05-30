#include "Extruder.h"
#include "Globals.h"

#include <Arduino.h>
#include <math.h>

float Extruder::getTemperature( uint8_t smoothed ) {
	uint16_t raw = analogRead( temp_pin );
	uint16_t r = smooth( raw );
	temperature = ABS_ZERO + beta / log( (r*Rs/(AD_RANGE - r)) /RInf );
	if( smoothed == 1 ) {
		return temperature;
	} else {
		return ABS_ZERO + beta / log( raw * Rs / (AD_RANGE - raw) / RInf );
	}
}

uint16_t Extruder::smooth( uint16_t new_value ) {
	if( temp_idx == 255 ) {
		memset( temps, new_value, sizeof( uint16_t ) * TEMPERATURE_SAMPLES );
		temp_idx = 0;
	} else {
		temps[ temp_idx ] = new_value;
		temp_idx = (temp_idx+1)%TEMPERATURE_SAMPLES;
	}
	uint16_t result = 0;
	for( uint8_t i = 0; i < TEMPERATURE_SAMPLES; i++ )
		result += temps[ i ] / TEMPERATURE_SAMPLES;
	return result;
}

void Extruder::calculateRInf() {
	RInf = NTC * exp( -beta / 298.15 );
}

uint8_t Extruder::autotune( float temp, int ncycles ) {
	float input = 0.0;
	int cycles = 0;
	bool heating = true;

	unsigned long temp_millis = millis(), t1 = temp_millis, t2 = temp_millis;
	long t_high = 0, t_low = 0;

	long bias, d;
	float Ku, Tu;
	float max = 0, min = 10000;
	int output = 0;

	digitalWrite( heater_pin, LOW );

	output = bias = d = PID_MAX / 2;

	// PID Tuning loop
	DEBUG_IO.println( "Beginning PID autotuning..." );
	for (;;) {
		unsigned long ms = millis();
		input = getTemperature(1);
		max = max(max, input);
		min = min(min, input);

		if (heating == true && input > temp) {
			if (ms - t2 > 5000) {
				heating = false;
				output = (bias - d) >> 1;
				t1 = ms;
				t_high = t1 - t2;
				max = temp;
			}
		}
		if (heating == false && input < temp) {
			if (ms - t1 > 5000) {
				DEBUG_IO.print( "Completed cycle " ); DEBUG_IO.println( cycles + 1 );
				heating = true;
				t2 = ms;
				t_low = t2 - t1;
				if (cycles > 0) {
					long max_pow = PID_MAX;
					bias += (d*(t_high - t_low))/(t_low + t_high);
					bias = constrain(bias, 20, max_pow - 20);
					d = (bias > max_pow / 2) ? max_pow - 1 - bias : bias;

					if (cycles > 2) {
						Ku = (4.0 * d) / (3.14159265 * (max - min) / 2.0);
						Tu = ((float)(t_low + t_high) / 1000.0);
						kp = 0.6 * Ku;
						ki = 2 * kp / Tu;
						kd = kp * Tu / 8;
						DEBUG_IO.print( "kU = " ); DEBUG_IO.print( Ku );
						DEBUG_IO.print( " Tu = " ); DEBUG_IO.print( Tu );
						DEBUG_IO.print( " kP = " ); DEBUG_IO.print( kp );
						DEBUG_IO.print( " kI = " ); DEBUG_IO.print( ki );
						DEBUG_IO.print( " kD = " ); DEBUG_IO.println( kd );
					}
				}
				output = (bias + d) >> 1;
				cycles++;
				min = temp;
			}
		}
		if (input > temp + 20) {
			DEBUG_IO.println( "Temperature exceeded target by 20C, cancelling autotuning" );
			return 0;
		}

		// Every 2 seconds...
		if (ms > temp_millis + 2000) {
			DEBUG_IO.print( input );
			DEBUG_IO.print( " @ " );
			DEBUG_IO.println( output );
			temp_millis = ms;
		} // every 2 seconds
		// Over 2 minutes?
		if (((ms - t1) + (ms - t2)) > (10L*60L*1000L*2L)) {
			DEBUG_IO.println( "PID autotuning timed out!" );
			return 0;
		}
		if (cycles > ncycles) {
			DEBUG_IO.println( "PID autotune finished!" );
			return 1;
		}
		if( output < 0 ) {
			digitalWrite( heater_pin, LOW );
		} else if( output >= 255 ) {
			digitalWrite( heater_pin, HIGH );
		} else {
			analogWrite( heater_pin, output );
		}
	}
}

void Extruder::runPID() {
	unsigned long dt = millis() - last;
	if( dt < PID_INTERVAL )
		return;

	float error = target_temperature - getTemperature( 1 );
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
