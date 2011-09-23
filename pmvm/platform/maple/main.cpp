/** \file
# This file is Copyright 2009 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
# 
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING up one directory from this.
*/

#include "stdlib.h"
#include "wirish.h"
#include "pmvm.h"
#include "plat.h"
#include "../../vm/pm.h"
#include "../../vm/global.h"
#include "pm.h"
#include "string.h"
#include "libmaple.h"
//#include "maple_plat.h"
#include "pymite_debug.h"

//Python needs its own heap, declare the size here
#define HEAP_SIZE 0x8000
//Serial port to communicate with Python
//#define SERIAL_COMM_PORT SerialUSB
//#define SERIAL_COMM_PORT Serial1
//#define SERIAL_COMM_PORT Serial2
#define SERIAL_COMM_PORT Serial3
//#define SERIAL_COMM_BAUD 9600
#define SERIAL_COMM_BAUD 115200



//#if SERIAL_COMM_PORT == SerialUSB
//	USBSerial *ser = NULL;	
//#else 
	HardwareSerial *ser = NULL;
//#endif 

extern unsigned char usrlib_img[];
static uint8_t main_heap[HEAP_SIZE] __attribute__((aligned((4))));;	

// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}

void setup(){
	//startup the comm port with the baudrate from above
//#if SERIAL_COMM_PORT == SerialUSB
//	ser->begin();
//#else
	ser->begin(SERIAL_COMM_BAUD);
//#endif
	
	//setup the button, this could be moved into pymite, but it's use seems so ubiquitous
	pinMode(BOARD_BUTTON_PIN, INPUT);

}



int main(void){

    PmReturn_t retval;
	//setup the communication port
	ser = &SERIAL_COMM_PORT;
	//set_comm_port(ser);

	//local setup
	setup();

	//initialize pymite
	retval = pm_init(main_heap, HEAP_SIZE, MEMSPACE_PROG, usrlib_img);
	PM_RETURN_IF_ERROR(retval);

	//run the script 'main.py'
	retval = pm_run((uint8_t *) "main");
	

	//for debug purposes if there is an error print out some useful info
	if (retval != PM_RET_OK){
		print_error(&main_heap[0], retval);
	}

	return 0;
}
