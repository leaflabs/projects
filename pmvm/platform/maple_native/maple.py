
"""__NATIVE__
#include "libmaple.h"
#include "gpio.h"
"""

def init_gpio():
	"""__NATIVE__
	PmReturn_t retval = PM_RET_OK;
	gpio_init(GPIOC);
	gpio_set_mode(GPIOC, 15, GPIO_OUTPUT_PP);
	NATIVE_SET_TOS(PM_NONE);
	return retval;
	"""
	pass

def toggle_led():
	"""__NATIVE__
	PmReturn_t retval = PM_RET_OK;
	gpio_toggle_bit(GPIOC, 15);
    NATIVE_SET_TOS(PM_NONE);
	return retval;
	"""
	pass

def test():
	"""__NATIVE__
	PmReturn_t retval = PM_RET_OK;
	NATIVE_SET_TOS(PM_NONE);
	return retval;
	"""

# :mode=c:
