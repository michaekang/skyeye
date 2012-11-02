#include "skyeye_module.h"

const char* skyeye_module = "skyeye_uart_term";

extern void init_uart_term();

void module_init(){
	init_uart_term();
}

void module_fini(){
}
