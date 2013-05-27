/*
 * skyeye_mach_am355x.c - define machine am355 for skyeye
 *
 * Copyright (C) 2013 Kewei Yu <keweihk@gmail.com>
 * Skyeye Develop Group, for help please send mail to
 * <skyeye-developer@lists.gro.clinux.org>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <stdlib.h>

#include <skyeye.h>
#include <skyeye_config.h>
#include <skyeye_mach.h>
#include <skyeye_arch.h>
#include <skyeye_types.h>
#include <skyeye_obj.h>
#include <skyeye_internal.h>
#include <skyeye_addr_space.h>

#include "am355x.h"

static void am355x_update_intr(void* mach) {
}

/**
* @brief Initialization of mach
*
* @param obj_name
*
* @return 
*/
static conf_object_t* new_am355x_mach(char* obj_name){
	am355x_mach_t* mach = skyeye_mm_zero(sizeof(am355x_mach_t));
	mach->obj = new_conf_object(obj_name, mach);
	mach->space = new_addr_space("am355x_mach_space");

	return mach->obj;
}

static void free_am355x_mach(conf_object_t* mach){
}

void init_am355x_mach(){
	static skyeye_class_t class_data = {
		.class_name = "am355x_mach",
		.class_desc = "am355x machine",
		.new_instance = new_am355x_mach,
		.free_instance = free_am355x_mach,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}

