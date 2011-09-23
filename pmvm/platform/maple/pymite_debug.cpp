//Variables for debug
//this could be put in main.cpp, but it clutters up the code
#include "stdint.h"
#include "wirish.h"
#include "pm.h"
#include "pymite_debug.h"

extern HardwareSerial *ser;
extern unsigned char usrlib_img[];
#define LEN_FNLOOKUP 26
#define LEN_EXNLOOKUP 18

#define S_SZ 200


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




void print_error(uint8_t *main_heap, PmReturn_t retval){
	uint8_t res;
    pPmFrame_t pframe;
    pPmObj_t pstr;
    uint8_t bcindex = 0;
//    uint16_t bcsum = 0;
    uint16_t linesum = 0;
//    uint16_t len_lnotab = 0;
//    uint8_t const *plnotab = NULL;
//    uint16_t i = 0;


	char string[S_SZ];
	char * str = &string[0];

	char string2[S_SZ];
	char * str2 = &string2[0];


	//let pm initialize everything
	/*
	   ser->println("");
	   ser->println("");
	   ser->print("usrlib_img address: \r\n\t0x" );
	   ser->println((int)usrlib_img, HEX);
	   ser->print("heap address: \r\n\t0x");
	   ser->println((int)main_heap, HEX);
	   ser->print("stack location: \r\n\t0x");
	   ser->println((int)STACK_TOP, HEX);
	   ser->print("TOS: \r\n\t0x");
	   ser->print((int)TOS, HEX);
	   ser->print(" = ");
	   ser->println((int)TOS->od, HEX);
	 */


	/* Get the top frame */
	pframe = gVmGlobal.pthread->pframe;

	snprintf (str, S_SZ,"Traceback (top first):\r\n");
	ser->print(str);

	// Get the top frame
	pframe = gVmGlobal.pthread->pframe;

	// If it's the native frame, print the native function name
	if (pframe == (pPmFrame_t)&(gVmGlobal.nativeframe)){

			ser->println("test");
			// The last name in the names tuple of the code obj is the name
			retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
							f_co->co_names, -1, &pstr);
			if ((retval) != PM_RET_OK){
					snprintf (str, S_SZ,"  Unable to get native func name.\r\n");
					ser->print(str);
			}
			else{
					snprintf (str, S_SZ,"  %s() __NATIVE__\r\n", ((pPmString_t)pstr)->val);
					ser->print(str);
			}

			// Get the frame that called the native frame
			pframe = (pPmFrame_t)gVmGlobal.nativeframe.nf_back;
	}

	// Print the remaining frame stack
	for (; pframe != C_NULL; pframe = pframe->fo_back) {
			ser->println("test2");
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
			ser->print(str);
	}
	res = (uint8_t)retval;
	if ((res > 0) && ((res - PM_RET_EX) < LEN_EXNLOOKUP)){
			snprintf (str, S_SZ, "%s, HEX Code: %X", exnlookup[res - PM_RET_EX], retval);
			ser->print(str);

	}

	ser->print(" detected by ");

	if ((gVmGlobal.errFileId > 0) && (gVmGlobal.errFileId < LEN_FNLOOKUP)){
			ser->print(fnlookup[gVmGlobal.errFileId]);
	}
	else{
			snprintf (str, S_SZ, "FileId 0x%02X line", gVmGlobal.errFileId);
			ser->print(str);
	}
	ser->print (" ");
	ser->print (gVmGlobal.errLineNum, DEC);
	ser->print ("\r\n");





	ser->println("Error detected");
	ser->print("Error: ");
	ser->println(retval, HEX);
	ser->print("\tRelease: ");
	ser->println(gVmGlobal.errVmRelease, HEX);
	ser->print("\tFileId: ");
	ser->print(gVmGlobal.errFileId, HEX);
	ser->print(" Name: ");
	ser->println(fnlookup[gVmGlobal.errFileId]);
	ser->print("\tLine Number: ");
	ser->println(gVmGlobal.errLineNum, DEC);


	ser->println("BUILTIN DICTIONARY:");
	ser->print("\t");
	ser->println((int)PM_PBUILTINS);



}
