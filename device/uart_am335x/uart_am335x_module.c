#include <skyeye_module.h>
#include <stdio.h>

const char *skyeye_module = "uart_am335x";

extern void init_am335x_uart();

void module_init(){
	init_am335x_uart();
}

void module_fini(){
}
