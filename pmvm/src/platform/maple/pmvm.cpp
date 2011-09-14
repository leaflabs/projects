
#include "pmvm.h"
#include "plat.h"
#include "wirish.h"
#include "timer.h"


#define US_UPDATE_RATE 500

static char read_char(){
	return SerialUSB.read();
}

static void write_char(char c){
	SerialUSB.write(c);
}

static unsigned int get_ms_ticks(){
	return millis();
}

static void timer_handler(){
	//report that 500us have passed	
	PMVM_us_timer_interrupt(US_UPDATE_RATE);	
}

PMVM::PMVM(){
	
	//I am the beginning
	PMVM::pm_opts = new (pm_opts_t);// malloc (sizeof (pm_opts_t));

	//attach the functions
	pm_opts->read_char = &read_char;
	pm_opts->write_char = &write_char;
	pm_opts->get_ms_ticks = &get_ms_ticks;

	//initialize the timer interrupt handler, send an update to pmvm every 500us
	//most of this was taken directly from examples/test-timers.cpp
	timer_init(TIMER1);
	timer_set_mode(TIMER1, 1, TIMER_OUTPUT_COMPARE); 
	timer_pause(TIMER1);
	uint32 cycles = US_UPDATE_RATE * CYCLES_PER_MICROSECOND; 
	uint16 pre = (uint16) ((cycles >> 16) + 1);
	timer_set_prescaler(TIMER1, pre);
	timer_set_reload(TIMER1, cycles/pre - 1);
	timer_attach_interrupt(TIMER1, TIMER_CC1_INTERRUPT, &timer_handler);
	timer_resume(TIMER1);
}


PMVM::~PMVM(){
	//I am the end :)
	delete(pm_opts);

	//cleanup and disable timer
	timer_pause(TIMER1);
	timer_detach_interrupt(TIMER1, timer_handler);
	timer_disable(TIMER1);
}
