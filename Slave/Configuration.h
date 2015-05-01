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
#define PID_INTERVAL 500 // milliseconds

#endif
