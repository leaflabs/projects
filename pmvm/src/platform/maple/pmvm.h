#ifndef __PMVM_INIT__
#define __PMVM_INIT__

#include "wirish.h"
#include "plat.h"
#include "timer.h"

class PMVM {
	
	public:
	pm_opts_t* pm_opts;

	private:
	public:
	PMVM();
	~PMVM();
};
void pmvm_init();
void pmvm_destroy();

#endif
