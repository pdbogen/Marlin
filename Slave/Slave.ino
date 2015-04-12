#include "Configuration.h"
#include "Globals.h"
#include "Init.h"
#include "Temperature.h"

void setup() {
	initialize_serial();
	initialize_temperatures();
	Serial.print( "Slave board initialized.\n" );  
}

void loop() {	
	check_hotend_temperatures();
	// check temperatures
	// process received commands
}
