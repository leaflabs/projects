/*
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


#include "pm.h"


#ifdef __cplusplus
extern
#endif

#define HEAP_SIZE 0x7000

unsigned char const usrlib_img[];


int
main(void)
{
    uint8_t heap[HEAP_SIZE];
    PmReturn_t retval;

    retval = pm_init(heap, HEAP_SIZE, MEMSPACE_PROG, usrlib_img);
    PM_RETURN_IF_ERROR(retval);

    /* Run the sample program */
    retval = pm_run((uint8_t *)"main");

    return (int)retval;
}
