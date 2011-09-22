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

#define LEN_FNLOOKUP 26
#define LEN_EXNLOOKUP 18



extern unsigned char usrlib_img[];


	/* This table should match src/vm/fileid.txt */
    char const * const fnlookup[LEN_FNLOOKUP] = {
        "<no file>",
        "codeobj.c",
        "dict.c",
        "frame.c",
        "func.c",
        "global.c",
        "heap.c",
        "img.c",
        "int.c",
        "interp.c",
        "pmstdlib_nat.c",
        "list.c",
        "main.c",
        "mem.c",
        "module.c",
        "obj.c",
        "seglist.c",
        "sli.c",
        "strobj.c",
        "tuple.c",
        "seq.c",
        "pm.c",
        "thread.c",
        "float.c",
        "class.c",
        "bytearray.c",
    };

    /* This table should match src/vm/pm.h PmReturn_t */
    char const * const exnlookup[LEN_EXNLOOKUP] = {
        "Exception",
        "SystemExit",
        "IoError",
        "ZeroDivisionError",
        "AssertionError",
        "AttributeError",
        "ImportError",
        "IndexError",
        "KeyError",
        "MemoryError",
        "NameError",
        "SyntaxError",
        "SystemError",
        "TypeError",
        "ValueError",
        "StopIteration",
        "Warning",
        "OverflowError",
    };



// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}

void setup(){
	Serial3.begin(115200);
	
	//setup the button
	pinMode(BOARD_BUTTON_PIN, INPUT);

}

#define HEAP_SIZE 0x7000

#define S_SZ 200

static uint8_t main_heap[HEAP_SIZE] __attribute__((aligned((4))));;	



int main(void){

	setup();
	uint8_t res;
    pPmFrame_t pframe;
    pPmObj_t pstr;
    PmReturn_t retval;
    uint8_t bcindex;
    uint16_t bcsum;
    uint16_t linesum;
    uint16_t len_lnotab;
    uint8_t const *plnotab;
    uint16_t i;


	char string[S_SZ];
	char * str = &string[0];

	char string2[S_SZ];
	char * str2 = &string2[0];



	//let pm initialize everything
	Serial3.println("");
	Serial3.println("Starting");
	Serial3.println("");
	Serial3.print("usrlib_img address: \r\n\t0x" );
	Serial3.println((int)usrlib_img, HEX);
	Serial3.print("heap address: \r\n\t0x");
	Serial3.println((int)main_heap, HEX);
	Serial3.print("stack address: \r\n\t0x");
	Serial3.println((int)STACK_TOP, HEX);
	

	retval = pm_init(main_heap, HEAP_SIZE, MEMSPACE_PROG, usrlib_img);
	Serial3.println(retval);
	PM_RETURN_IF_ERROR(retval);
	
	retval = pm_run((uint8_t *) "main");
	
	Serial3.println("Exit PYMITE");

	if (retval != PM_RET_OK){

			//let pm initialize everything
			Serial3.println("");
			Serial3.println("");
			Serial3.print("usrlib_img address: \r\n\t0x" );
			Serial3.println((int)usrlib_img, HEX);
			Serial3.print("heap address: \r\n\t0x");
			Serial3.println((int)main_heap, HEX);
			Serial3.print("stack location: \r\n\t0x");
			Serial3.println((int)STACK_TOP, HEX);
/*
	
			debug_names[0] = strdup("Heap Base");
			debug_names[1] = strdup("DEBUG1");
			debug_names[2] = strdup("DEBUG2");
			debug_names[3] = strdup("DEBUG3");
			debug_names[4] = strdup("TOP IP");


			Serial3.println("Debug Data: ");
			for (i = 0; i < DEBUG_SIZE; i++){
					Serial3.print("\t");
					Serial3.print((const char *)debug_names[i]);
					Serial3.print(":\r\n\t\t0x");
					Serial3.println(debug_data[i], HEX);
					Serial3.print("\t\t");
					Serial3.println(debug_data[i], DEC);
			}
*/
			/* Get the top frame */
			pframe = gVmGlobal.pthread->pframe;

			snprintf (str, S_SZ,"Traceback (top first):\r\n");
			Serial3.print(str);

			// Get the top frame
			pframe = gVmGlobal.pthread->pframe;

			// If it's the native frame, print the native function name
			if (pframe == (pPmFrame_t)&(gVmGlobal.nativeframe)){

				Serial3.println("test");
					// The last name in the names tuple of the code obj is the name
					retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
									f_co->co_names, -1, &pstr);
					if ((retval) != PM_RET_OK){
							snprintf (str, S_SZ,"  Unable to get native func name.\r\n");
							Serial3.print(str);
					}
					else{
							snprintf (str, S_SZ,"  %s() __NATIVE__\r\n", ((pPmString_t)pstr)->val);
							Serial3.print(str);
					}

					// Get the frame that called the native frame
					pframe = (pPmFrame_t)gVmGlobal.nativeframe.nf_back;
			}

			// Print the remaining frame stack
			for (; pframe != C_NULL; pframe = pframe->fo_back) {
				Serial3.println("test2");
					// The last name in the names tuple of the code obj is the name
					retval = tuple_getItem((pPmObj_t)pframe->fo_func->f_co->co_names, -1, 
									&pstr);
					if ((retval) != PM_RET_OK) break;

					bcindex = pframe->fo_ip - pframe->fo_func->f_co->co_codeaddr;
					/*
					   plnotab = pframe->fo_func->f_co->co_lnotab;
					   len_lnotab = mem_getWord(MEMSPACE_PROG, &plnotab);
					   bcsum = 0;
					   linesum = pframe->fo_func->f_co->co_firstlineno;


					   for (i = 0; i < len_lnotab; i += 2){
					   bcsum += mem_getByte(MEMSPACE_PROG, &plnotab);
					   if (bcsum > bcindex) break;
					   linesum += mem_getByte(MEMSPACE_PROG, &plnotab);
					   }

					//get the filename of thi frame's function
					if (((pPmFrame_t)pframe)->fo_func->f_co->co_memspace == MEMSPACE_PROG) {
					snprintf (str2, S_SZ, (char *) ((pPmFrame_t)pframe)->fo_func->f_co->co_filename);
					}
					 */

					snprintf (str, S_SZ, "File: %s, line: %d, in: %s, byte code address: 0x%X Value: %X\r\n", 
									str2,
									linesum,				
									((pPmString_t)pstr)->val,
									(unsigned int) ((pPmFrame_t)pframe)->fo_ip,
									(unsigned int) *((pPmFrame_t)pframe)->fo_ip
							 );
					Serial3.print(str);
			}
			res = (uint8_t)retval;
			if ((res > 0) && ((res - PM_RET_EX) < LEN_EXNLOOKUP)){
					snprintf (str, S_SZ, "%s, HEX Code: %X", exnlookup[res - PM_RET_EX], retval);
					Serial3.print(str);

			}

			Serial3.print(" detected by ");

			if ((gVmGlobal.errFileId > 0) && (gVmGlobal.errFileId < LEN_FNLOOKUP)){
					Serial3.print(fnlookup[gVmGlobal.errFileId]);
			}
			else{
					snprintf (str, S_SZ, "FileId 0x%02X line", gVmGlobal.errFileId);
					Serial3.print(str);
			}
			Serial3.print (" ");
			Serial3.print (gVmGlobal.errLineNum, DEC);
			Serial3.print ("\r\n");





			Serial3.println("Error detected");
			Serial3.print("Error: ");
			Serial3.println(retval, HEX);
			Serial3.print("\tRelease: ");
			Serial3.println(gVmGlobal.errVmRelease, HEX);
			Serial3.print("\tFileId: ");
			Serial3.print(gVmGlobal.errFileId, HEX);
			Serial3.print(" Name: ");
			Serial3.println(fnlookup[gVmGlobal.errFileId]);
			Serial3.print("\tLine Number: ");
			Serial3.println(gVmGlobal.errLineNum, DEC);


			Serial3.println("BUILTIN DICTIONARY:");
			Serial3.print("\t");
			Serial3.println((int)PM_PBUILTINS);

	}

	return 0;
}
