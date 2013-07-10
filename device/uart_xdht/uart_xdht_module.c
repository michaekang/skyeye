#include <skyeye_module.h>
#include <stdio.h>

const char *skyeye_module = "uart_xdht";

extern void init_xdht_uart();

void module_init(){
	init_xdht_uart();
}

void module_fini(){
}
