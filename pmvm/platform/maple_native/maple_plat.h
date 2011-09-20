/*
	here are all the C++ header things that can't exists in the plat.h
*/


#ifndef __MAPLE_PLAT__
#define __MAPLE_PLAT__

#include "comm/HardwareSerial.h"

void set_comm_port (HardwareSerial *ser);

#endif
