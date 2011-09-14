/*
# This file is Copyright 2010 Dean Hall.
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

#ifndef _PLAT_H_
#define _PLAT_H_

#define PM_HEAP_SIZE 0x7000
#define PM_FLOAT_LITTLE_ENDIAN

typedef struct _pm_opts_t pm_opts_t;

struct _pm_opts_t {
	char (*read_char) ();
	void (*write_char) (char c);
	unsigned int (*get_ms_ticks) ();
};


void PMVM_init(pm_opts_t * pm_opts);

void PMVM_us_timer_interrupt(unsigned int us);

#endif /* _PLAT_H_ */
