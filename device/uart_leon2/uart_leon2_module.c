#include "skyeye_module.h"

const char* skyeye_module = "uart_leon2";

extern void init_leon2_uart();

void module_init(){
	init_leon2_uart();
}

void module_fini(){
}
