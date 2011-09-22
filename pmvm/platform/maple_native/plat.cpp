/** @file
 * PyMite - A flyweight Python interpreter for 8-bit and larger microcontrollers.
 * Copyright 2002 Dean Hall.  All rights reserved.
 * PyMite is offered through one of two licenses: commercial or open-source.
 * See the LICENSE file at the root of this package for licensing details.
 *
 * some sections based on code (C) COPYRIGHT 2008 STMicroelectronics
 */


#undef __FILE_ID__
#define __FILE_ID__ 0x70

#include "maple_plat.h"
#include "stdio.h"
#include "string.h"
#include "pm.h"
#include "obj.h"
#include "wirish.h"
#include "comm/HardwareSerial.h"


HardwareSerial *s;


#define US_UPDATE_RATE 60000

/*
 * Retargets the C library printf function to the USART.
 */
int fputc(int ch, FILE *f){
    plat_putByte((uint8_t) ch);
    return ch;
}

static void timer_handler(){
	//s->println("t");
	//report that 500us have passed	
	pm_vmPeriodic(US_UPDATE_RATE);
}


void set_comm_port (HardwareSerial *ser){
	s = ser;	
}

PmReturn_t plat_init(void) {
	s = &Serial3;
	timer_init(TIMER1);
	timer_set_mode(TIMER1, 1, TIMER_OUTPUT_COMPARE); 
	timer_pause(TIMER1);
	uint32 cycles = US_UPDATE_RATE * CYCLES_PER_MICROSECOND; 
	uint16 pre = (uint16) ((cycles >> 16) + 1);
	timer_set_prescaler(TIMER1, pre);
	timer_set_reload(TIMER1, cycles/pre - 1);
	timer_attach_interrupt(TIMER1, TIMER_CC1_INTERRUPT, &timer_handler);
	timer_resume(TIMER1);

    return PM_RET_OK;
}

PmReturn_t plat_deinit(void){
	timer_pause(TIMER1);
	timer_detach_interrupt(TIMER1, 1);
	timer_disable(TIMER1);

    return PM_RET_OK;
}


/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr){
	//get a byte from memory
	
    uint8_t b = 0;
//	s->print((int)memspace, DEC);
//	s->print(" ");

    switch (memspace)
    {
        case MEMSPACE_RAM:
        case MEMSPACE_PROG:
            b = **paddr;
            *paddr += 1;
//			s->print("0x");
//			s->print((int)*paddr, HEX);
//			s->print(" : ");
//			s->println((uint8)**paddr, HEX);
            return b;
        case MEMSPACE_EEPROM:
        case MEMSPACE_SEEPROM:
        case MEMSPACE_OTHER0:
        case MEMSPACE_OTHER1:
        case MEMSPACE_OTHER2:
        case MEMSPACE_OTHER3:
        default:
			s->println ("ERROR!");
            return 0;
    }
	
	return 0;
}


PmReturn_t plat_getByte(uint8_t *b){

    PmReturn_t retval = PM_RET_OK;
	while (!s->available()){}
	*b = s->read();
    return retval;
}


PmReturn_t plat_putByte(uint8_t b){

//	s->print("in put byte");
	s->print((char)b);
    return PM_RET_OK;
}


PmReturn_t plat_getMsTicks(uint32_t *r_ticks){
	//s->print("time check");
	*r_ticks =  millis();
    return PM_RET_OK;
}

#define S_SZ 200

void plat_reportError(PmReturn_t result) {

	char string[S_SZ];
	char * str = &string[0];

	char string2[S_SZ];
	char * str2 = &string2[0];
	
#ifdef HAVE_DEBUG_INFO
#define LEN_FNLOOKUP 26
#define LEN_EXNLOOKUP 18

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


	/* Get the top frame */
    pframe = gVmGlobal.pthread->pframe;

	snprintf (str, S_SZ,"Traceback (top first):\r\n");
	s->print(str);

	// Get the top frame
	pframe = gVmGlobal.pthread->pframe;

	// If it's the native frame, print the native function name
	if (pframe == (pPmFrame_t)&(gVmGlobal.nativeframe)){

		// The last name in the names tuple of the code obj is the name
		retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
                                   f_co->co_names, -1, &pstr);
		if ((retval) != PM_RET_OK){
				snprintf (str, S_SZ,"  Unable to get native func name.\r\n");
				s->print(str);
                return;
        }
        else{
             	snprintf (str, S_SZ,"  %s() __NATIVE__\r\n", ((pPmString_t)pstr)->val);
				s->print(str);
		}

       	// Get the frame that called the native frame
		pframe = (pPmFrame_t)gVmGlobal.nativeframe.nf_back;
	}

    // Print the remaining frame stack
	for (; pframe != C_NULL; pframe = pframe->fo_back) {
            // The last name in the names tuple of the code obj is the name
            retval = tuple_getItem((pPmObj_t)pframe->fo_func->f_co->co_names, -1, 
											&pstr);
            if ((retval) != PM_RET_OK) break;

		    bcindex = pframe->fo_ip - pframe->fo_func->f_co->co_codeaddr;
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
			
            snprintf (str, S_SZ, "File: %s, line: %d, in: %s, byte code address: 0x%X Value: %X\r\n", 
				str2,
				linesum,				
				((pPmString_t)pstr)->val,
				(unsigned int) ((pPmFrame_t)pframe)->fo_ip,
				(unsigned int) *((pPmFrame_t)pframe)->fo_ip
				);
			s->print(str);
		}
	    res = (uint8_t)result;
	    if ((res > 0) && ((res - PM_RET_EX) < LEN_EXNLOOKUP)){
			snprintf (str, S_SZ, "%s, HEX Code: %X", exnlookup[res - PM_RET_EX], result);
			s->print(str);
			
	    }
	    else{
			s->print ("Error code");
			s->print (result, HEX);
	    }

		if (res == PM_RET_EX_TYPE){
			s->print ("\r\n");
			snprintf (str, S_SZ, "Type Error: 0x%X != 0x%X\r\n", OBJ_TYPE_DIC, OBJ_GET_TYPE(*((pPmFrame_t)pframe)->fo_ip)); 
			s->print(str);
		}
	    s->print(" detected by ");

	    if ((gVmGlobal.errFileId > 0) && (gVmGlobal.errFileId < LEN_FNLOOKUP)){
			s->print(fnlookup[gVmGlobal.errFileId]);
	    }
	    else{
			snprintf (str, S_SZ, "FileId 0x%02X line", gVmGlobal.errFileId);
			s->print(str);
	    }
		s->print (" ");
		s->print (gVmGlobal.errLineNum, DEC);
		s->print ("\r\n");



/*
		snprintf (str, S_SZ, "Error:\t0x%02X\r\n", result);
		s->print(str);
	    snprintf (str, S_SZ, "\tRelease:\t0x%02X\r\n", gVmGlobal.errVmRelease);
		s->print(str);
	    snprintf (str, S_SZ,  "\tFileId:\t0x%02X\r\n", gVmGlobal.errFileId);
		s->print(str);
	    snprintf (str, S_SZ, "\tLineNum:\t%d\r\n", gVmGlobal.errLineNum);
		s->print(str);
*/
#endif

}



