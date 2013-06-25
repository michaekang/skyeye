/*
 * skyeye_mach_am335x.c - define machine am335 for skyeye
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

#include "am335x.h"
exception_t set_conf_attr(conf_object_t* obj, char* attr_name, attr_value_t* value);

static void am335x_update_intr(void* mach) {
}

/**
* @brief Initialization of mach
*
* @param obj_name
*
* @return 
*/
static conf_object_t* new_am335x_mach(char* obj_name){
	am335x_mach_t* mach = skyeye_mm_zero(sizeof(am335x_mach_t));
	mach->obj = new_conf_object(obj_name, mach);

	conf_object_t* arm_cpu = pre_conf_obj("arm_cpu0", "arm_cpu");
	exception_t ret = reset_conf_obj(arm_cpu);
	mach->space = (addr_space_t*)SKY_get_interface(arm_cpu, ADDR_SPACE_INTF_NAME);

	/* instance a image class */
	conf_object_t* image0 = pre_conf_obj("image0", "image");
	attr_value_t* value = make_new_attr(Val_UInteger, 0x50000000);
	ret = set_conf_attr(image0, "size", value);
	/* instance a ram class */
	conf_object_t* ram0 = pre_conf_obj("ram0", "ram");
	value = make_new_attr(Val_Object, image0);
	ret = set_conf_attr(ram0, "image", value);

	memory_space_intf* ram0_io_memory = (memory_space_intf*)SKY_get_interface(ram0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(mach->space, 0x50000000, 0x50000000, 0x0, ram0_io_memory, 1, 1);

	conf_object_t* intc0 = pre_conf_obj("intc_am335x_0", "intc_am335x");
	memory_space_intf* intc0_io_memory = (memory_space_intf*)SKY_get_interface(intc0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(mach->space, 0x48200000, 0xfff, 0x0, intc0_io_memory, 1, 1);

	conf_object_t* timer2 = pre_conf_obj("timer2_am335x", "timer_am335x");
	memory_space_intf* timer2_io_memory = (memory_space_intf*)SKY_get_interface(timer2, MEMORY_SPACE_INTF_NAME);
	ret = add_map(mach->space, 0x48040000, 0x58, 0x0, timer2_io_memory, 1, 1);

	conf_object_t* prcm0 = pre_conf_obj("prcm_am335x_0", "prcm_am335x");
	memory_space_intf* prcm0_io_memory = (memory_space_intf*)SKY_get_interface(prcm0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(mach->space, 0x44e00000, 0xaff, 0x0, prcm0_io_memory, 1, 1);
	ret = reset_conf_obj(prcm0);

	return mach->obj;
}

static void free_am335x_mach(conf_object_t* mach){
}

void init_am335x_mach(){
	static skyeye_class_t class_data = {
		.class_name = "am335x_mach",
		.class_desc = "am335x machine",
		.new_instance = new_am335x_mach,
		.free_instance = free_am335x_mach,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}

