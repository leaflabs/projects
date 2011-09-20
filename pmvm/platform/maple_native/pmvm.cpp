
#include "pmvm.h"
#include "plat.h"
#include "wirish.h"
#include "timer.h"


#define US_UPDATE_RATE 500

static pm_opts_t pm_opts;

volatile int count = 0;


static char read_char(){
	return Serial3.read();
}

static void write_char(char c){
	Serial3.write(c);
}

static unsigned int get_ms_ticks(){
	return millis();
}

void write_string (char *string){
	Serial3.print(string);
}
#define INIT_TEST 0
#define PMVM_ERROR 1
#define PMVM_INIT 2
#define PMVM_DEINIT 3
#define PMVM_GET_BYTE 4
#define PMVM_MEM_ERRORA 5
#define PMVM_MEM_ERRORB 6
#define PMVM_FPUTC 7
#define PMVM_FGETC 8
#define PMVM_GET_TICKS 9
#define PMVM_MEM_ACCESS 10

static void error_bom(int index){
	switch (index){
		case (INIT_TEST):
//			Serial3.println("in error_bom test");
			break;
		case (PMVM_ERROR):
//			Serial3.println("PMVM is trying to report an error");
			break;
		case (PMVM_INIT):
//			Serial3.println("PMVM init");
			break;
		case (PMVM_DEINIT):
//			Serial3.println("PMVM deinit");
			break;
		case (PMVM_GET_BYTE):
			count++;
//			Serial3.println(count, DEC);
//			Serial3.print(".");
			break;
		case (PMVM_MEM_ERRORA):
//			Serial3.println("mem errora");
			break;
		case (PMVM_MEM_ERRORB):
//			Serial3.println("mem errorb");
			break;
		case (PMVM_FPUTC):
//			Serial3.println("fputc called");
			break;
		case (PMVM_FGETC):
//			Serial3.println("fgetc called");
			break;
		case (PMVM_GET_TICKS):
//			Serial3.println("get ticks called");
			break;
		case (PMVM_MEM_ACCESS):
//			Serial3.print("m");

		default:
			Serial3.print (index, HEX);
			char * p = (char *) index;
			Serial3.print (":");
			Serial3.println ((uint8)*p, HEX);
			break;
	}
}
static void timer_handler(){
	//report that 500us have passed	
	PMVM_us_timer_interrupt(US_UPDATE_RATE);	
}


void maple_pmvm_init (){
	pm_opts_t * pm_opts_p = &pm_opts;
	pm_opts.read_char = &read_char;
	pm_opts.write_char = &write_char;
	pm_opts.get_ms_ticks = &get_ms_ticks;
	pm_opts.error_bom = &error_bom;
	pm_opts.write_string = &write_string;

	PMVM_init(pm_opts_p);

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

void maple_pmvm_deinit(){
	timer_pause(TIMER1);
	timer_detach_interrupt(TIMER1, 1);
	timer_disable(TIMER1);

}
//}


//PMVM::~PMVM(){
//	//I am the end :)
//	delete(pm_opts);
//
//	//cleanup and disable timer

//	timer_pause(TIMER1);
//	timer_detach_interrupt(TIMER1, 1);
//	timer_disable(TIMER1);

//}
