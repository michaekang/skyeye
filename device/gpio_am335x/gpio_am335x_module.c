#include <skyeye_module.h>
#include <stdio.h>

const char *skyeye_module = "gpio_am335x";

extern void init_am335x_gpio();

void module_init(){
	init_am335x_gpio();
}

void module_fini(){
}
