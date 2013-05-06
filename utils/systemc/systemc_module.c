#include <stdio.h>
#include <errno.h>
#include "skyeye_module.h"

void init_systemc_class();
/* module name */
const char* skyeye_module = "systemc";

/* module initialization and will be executed automatically when loading. */
void module_init(){
	init_systemc_class();
}

/* module destruction and will be executed automatically when unloading */
void module_fini(){
}
