/* Copyright (C) 
* 2013 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file c6k_module.c
* @brief The module of c6k
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#include <stdlib.h>
#include "skyeye_module.h"
#include "skyeye_mach.h"

const char* skyeye_module = "c6k";

extern void init_c6k_arch ();
extern void c6747_mach_init();
extern void c6747_mach_fini();
extern void c6713_mach_init();
extern void c6713_mach_fini();

machine_config_t c6k_machines[] = {
	{"c6747", c6747_mach_init, NULL, NULL, NULL},
	{"c6713", c6713_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};

void module_init(){
	//skyeye_module = strdup("c6000");
	init_c6k_arch ();
	/*
         * register all the supported mach to the common library.
         */
	int i = 0;
	extern machine_config_t c6k_machines[];
        while(c6k_machines[i].machine_name != NULL){
                register_mach(c6k_machines[i].machine_name, c6k_machines[i].mach_init);
                i++;
        }

}

void module_fini(){
}
