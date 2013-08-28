/* Copyright (C) 
* 
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
* @file can_sja1000.c
* @brief The implementation of system controller
* @author 
* @version 78.77
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
#include <skyeye_can_ops.h>
#define DEBUG
#include <skyeye_log.h>

#include "can_sja1000.h"

static exception_t can_sja1000_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct can_sja1000_device *dev = opaque->obj;
	can_sja1000_reg_t* regs = dev->regs;
	switch(offset) {
		case 0x0:
			*(uint8_t *)buf = regs->mode;
			break;
		case 0x4:
			*(uint8_t *)buf = regs->ier;
			break;
		case 0x8:
			*(uint8_t *)buf = regs->output_ctrl;
			break;
		case 0xc:
			*(uint8_t *)buf = regs->error_code;
			break;
		default:
			printf("Can not read the register at 0x%x in can_sja1000\n", offset);
			return Invarg_exp;
	}
	return No_exp;

}

static exception_t can_sja1000_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct can_sja1000_device *dev = opaque->obj;
	can_sja1000_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	switch(offset) {
		case 0x0:
			regs->mode = val;
			break;
		case 0x4:
			regs->ier = val;
			break;
		case 0x8:
                        regs->output_ctrl = val;
                        break;
                case 0xc:
			regs->error_code = val;
                        break;

		default:
			printf("Can not write the register at 0x%x in can_sja1000\n", offset);
			return Invarg_exp;
	}
	return No_exp;

}

static char* sja1000_get_regname_by_id(conf_object_t* conf_obj, uint32_t id)
{
	struct can_sja1000_device* dev = conf_obj->obj;
	return NULL;
}

static uint32_t sja1000_get_regval_by_id(conf_object_t* conf_obj, uint32_t id)
{
	struct can_sja1000_device* dev = conf_obj->obj;
	uint32_t* regs_value = (uint32_t*)(dev->regs) + id;
	return *regs_value;
}

static conf_object_t* new_can_sja1000(char* obj_name){
	can_sja1000_device* dev = skyeye_mm_zero(sizeof(can_sja1000_device));
	can_sja1000_reg_t* regs =  skyeye_mm_zero(sizeof(can_sja1000_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	/* init can_sja1000 regs */
	dev->regs = regs;

	can_ops_intf* ops = skyeye_mm_zero(sizeof(can_ops_intf));
	dev->can_ops = ops;
	ops->obj = NULL;
	ops->start = NULL;
	ops->stop = NULL;
	ops->transmit = NULL;
	ops->receive = NULL;
	SKY_register_interface(dev->can_ops, obj_name, CAN_OPS_INTF_NAME);	

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = can_sja1000_read;
	io_memory->write = can_sja1000_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);

	/* register the interface for registers */
	skyeye_reg_intf* reg_intf = skyeye_mm_zero(sizeof(skyeye_reg_intf));
	reg_intf->conf_obj = dev->obj;
	reg_intf->get_regvalue_by_id = sja1000_get_regval_by_id;
	reg_intf->get_regname_by_id = sja1000_get_regname_by_id;
	reg_intf->set_regvalue_by_id = NULL;
	SKY_register_interface((void*)reg_intf, obj_name, SKYEYE_REG_INTF);

	return dev->obj;
}
void free_can_sja1000(conf_object_t* dev){
	
}

void init_can_sja1000(){
	static skyeye_class_t class_data = {
		.class_name = "can_sja1000",
		.class_desc = "can_sja1000",
		.new_instance = new_can_sja1000,
		.free_instance = free_can_sja1000,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}
