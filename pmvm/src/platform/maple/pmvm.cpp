
#include "pmvm.h"
#include "plat.h"
#include "wirish.h"



static char read_char(){
	return SerialUSB.read();
}

static void write_char(char c){
	SerialUSB.write(c);
}

static unsigned int get_ms_ticks(){
	return millis();
}

PMVM::PMVM(){
	
	//I am the beginning
	PMVM::pm_opts = new (pm_opts_t);// malloc (sizeof (pm_opts_t));

	//attach the functions
	pm_opts->read_char = &read_char;
	pm_opts->write_char = &write_char;
	pm_opts->get_ms_ticks = &get_ms_ticks;

}

PMVM::~PMVM(){
	//I am the end :)
	delete(pm_opts);
}
