/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
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
* @file am35x_hecc.c
* @brief The implementation of system controller
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-12-12
*/

#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h> 
#include <memory_space.h>
#include <skyeye_device.h>
#define DEBUG
#include <skyeye_log.h>

#include "skyeye_can_ops.h"
#include "am35x_hecc.h"

static exception_t am35x_hecc_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct am35x_hecc_device *dev = opaque->obj;
	hecc_reg_t* regs = dev->regs;
	switch(offset) {
		case 0x0:
			*(uint32_t*)buf = regs->dcan_ctl;
			break;		
		case 0x4:
			*(uint32_t*)buf = regs->dcan_es;
			break;

		default:
			printf("Can not read the register at 0x%x in hecc\n", offset);
			return Invarg_exp;
	}
	return No_exp;
}

static exception_t am35x_hecc_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct am35x_hecc_device *dev = opaque->obj;
	hecc_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	switch(offset) {
		case 0x100:
			regs->if1_cmd = val;
			int msg_num = regs->if1_cmd & 0xFF;
			if(msg_num > 0x1 && msg_num < 0x81){ /* valid msg number */
				/* transfer is triggered */
				regs->if1_cmd |= 0x8000; /* set BUSY bit */
			}
		default:
			printf("Can not write the register at 0x%x in hecc\n", offset);
			return Invarg_exp;
	}
	return No_exp;
}
static conf_object_t* new_am35x_hecc(char* obj_name){
	am35x_hecc_device* dev = skyeye_mm_zero(sizeof(am35x_hecc_device));
	hecc_reg_t* regs =  skyeye_mm_zero(sizeof(hecc_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	dev->regs = regs;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = am35x_hecc_read;
	io_memory->write = am35x_hecc_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	
	return dev->obj;
}
void free_am35x_hecc(conf_object_t* dev){
	
}

void init_am35x_hecc(){
	static skyeye_class_t class_data = {
		.class_name = "am35x_hecc",
		.class_desc = "HECC of TI am35x",
		.new_instance = new_am35x_hecc,
		.free_instance = free_am35x_hecc,
		.get_attr = NULL,
		.set_attr = NULL
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}