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

#include "wirish.h"
//#include "pm.h"

extern unsigned char usrlib_img[];
// Force init to be called *first*, i.e. before static object allocation.
// Otherwise, statically allocated objects that need libmaple may fail.
__attribute__((constructor)) void premain() {
    init();
}


int
main(void)
{
/*
    PmReturn_t retval;

    // Init PyMite
    retval = pm_init(MEMSPACE_PROG, usrlib_img);
    PM_RETURN_IF_ERROR(retval);

    // Run the sample program
    retval = pm_run((uint8_t *)"main");


    return (int)retval;
	*/
}
