#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#define DEBUG_IO Serial
#define MASTER   Serial1

#define HOT_ENDS 1

#define BETAS { 3960ul }
#define RS { 4700ul }
#define NTC 100000ul

#define AD_RANGE 1023.0

#define PACKET_QUEUE_SIZE 10

//#define DEBUG_PID
#define PID_INTERVAL 100 // milliseconds

//#define DEBUG_SERIAL_IO

#define TEMPERATURE_SAMPLES 5
#define PREV_FACTOR 10.0
#define TEMP_SMOOTHED 1
#define TEMP_RAW 0

#define PID_MAX 255
#endif
