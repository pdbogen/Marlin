#include "Configuration.h"
#include "Globals.h"
#include "Init.h"
#include "Interrupt.h"
#include "Temperature.h"
#include "Communications.h"

void setup() {
	initialize_interrupts();
	initialize_serial();
	initialize_temperatures();
	Serial.print( "Slave board initialized.\n" );
}

void loop() {
	check_hotend_temperatures();
	run_hotend_pid();
	network_loop();
}
