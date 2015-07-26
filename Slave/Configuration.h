#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define DEBUG_IO    Serial
#define DEBUG_BAUD  115200
#define MASTER      Serial1
#define MASTER_BAUD 57600

#define HOT_ENDS 1

#define BETAS { 3960ul }
#define RS { 4700ul }
#define NTC 100000ul

#define AD_RANGE 1023.0

#define PACKET_QUEUE_SIZE 10

//#define DEBUG_PID
#define PID_INTERVAL 100 // milliseconds

//#define DEBUG_SERIAL_IO

#define PID_MAX 255

//These defines help to calibrate the AD595 sensor in case you get wrong temperature measurements.
//The measured temperature is defined as "actualTemp = (measuredTemp * TEMP_SENSOR_AD595_GAIN) + TEMP_SENSOR_AD595_OFFSET"
#define TEMP_SENSOR_AD595_OFFSET 0.0
#define TEMP_SENSOR_AD595_GAIN   1.0

#define THERMISTORHEATER_0 71 // Honeywell NTC 5% 100KOhm 135-104LAF-J01
#endif
