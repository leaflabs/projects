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

#include "stdio.h"
#include "../../vm/pm.h"
#include "libmaple.h"

//GLOBAL VARIABLES YAY!!!!
pm_opts_t * opts;


/*
 * Retargets the C library printf function to the USART.
 */
int fputc(int ch, FILE *f){
    plat_putByte((uint8_t) ch);
    return ch;
}


PmReturn_t plat_init(void) {
	//everything was configured in wirish
    return PM_RET_OK;
}

PmReturn_t plat_deinit(void){
    return PM_RET_OK;
}


/*
 * Gets a byte from the address in the designated memory space
 * Post-increments *paddr.
 */
uint8_t plat_memGetByte(PmMemSpace_t memspace, uint8_t const **paddr){
	//get a byte from memory
	
    uint8_t b = 0;

    switch (memspace)
    {
        case MEMSPACE_RAM:
        case MEMSPACE_PROG:
            b = **paddr;
            *paddr += 1;
            return b;
        case MEMSPACE_EEPROM:
        case MEMSPACE_SEEPROM:
        case MEMSPACE_OTHER0:
        case MEMSPACE_OTHER1:
        case MEMSPACE_OTHER2:
        case MEMSPACE_OTHER3:
        default:
            return 0;
    }
	
	return 0;
}


PmReturn_t plat_getByte(uint8_t *b){

    int c;
    PmReturn_t retval = PM_RET_OK;
	c = opts->read_char();
    return retval;
}


PmReturn_t plat_putByte(uint8_t b){

	opts->write_char((char) b);

    return PM_RET_OK;
}


PmReturn_t plat_getMsTicks(uint32_t *r_ticks){
    *r_ticks = (uint32_t) opts->get_ms_ticks();
    return PM_RET_OK;
}


void plat_reportError(PmReturn_t result) {

    //Print error
    printf("Error:     0x%02X\n", result);
    printf("  Release: 0x%02X\n", gVmGlobal.errVmRelease);
    printf("  FileId:  0x%02X\n", gVmGlobal.errFileId);
    printf("  LineNum: %d\n", gVmGlobal.errLineNum);

    //Print traceback
    {
        pPmObj_t pframe;
        pPmObj_t pstr;
        PmReturn_t retval;

        printf("Traceback (top first):\n");

        // Get the top frame
        pframe = (pPmObj_t)gVmGlobal.pthread->pframe;

        // If it's the native frame, print the native function name
        if (pframe == (pPmObj_t)&(gVmGlobal.nativeframe))
        {

            // The last name in the names tuple of the code obj is the name
            retval = tuple_getItem((pPmObj_t)gVmGlobal.nativeframe.nf_func->
                                   f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK)
            {
                printf("  Unable to get native func name.\n");
                return;
            }
            else
            {
                printf("  %s() __NATIVE__\n", ((pPmString_t)pstr)->val);
            }

            // Get the frame that called the native frame
            pframe = (pPmObj_t)gVmGlobal.nativeframe.nf_back;
        }

        // Print the remaining frame stack
        for (;
             pframe != C_NULL;
             pframe = (pPmObj_t)((pPmFrame_t)pframe)->fo_back)
        {
            // The last name in the names tuple of the code obj is the name
            retval = tuple_getItem((pPmObj_t)((pPmFrame_t)pframe)->
                                   fo_func->f_co->co_names, -1, &pstr);
            if ((retval) != PM_RET_OK) break;

            printf("  %s()\n", ((pPmString_t)pstr)->val);
        }
        printf("  <module>.\n");
    }
}


void PMVM_init(pm_opts_t * pm_opts){
	opts = pm_opts;	
}

