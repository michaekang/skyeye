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
* @file prcm_am335x.c
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
#define DEBUG
#include <skyeye_log.h>

#include "prcm_am335x.h"

static exception_t prcm_am335x_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct prcm_am335x_device *dev = opaque->obj;
	prcm_am335x_reg_t* regs = dev->regs;
	switch(offset) {
		case CM_PER_L4LS_CLKSTCTRL:
			*(uint32_t*)buf = regs->per.l4ls_clkstctrl;
			break;
		case CM_PER_L3S_CLKSTCTRL:
			*(uint32_t*)buf = regs->per.l3s_clkstctrl;
			break;
		case CM_PER_L3_CLKSTCTRL:
			*(uint32_t*)buf = regs->per.l3_clkstctrl;
			break;
		case CM_PER_L4LS_CLKCTRL:
			*(uint32_t*)buf = regs->per.l4ls_clkctrl;
			break;
		case CM_PER_L3_INSTR_CLKCTRL:
			*(uint32_t*)buf = regs->per.l3_instr_clkctrl;
			break;
		case CM_PER_L3_CLKCTRL:
			*(uint32_t*)buf = regs->per.l3_clkctrl;
			break;
		case CM_PER_OCPWP_L3_CLKSTCTRL:
			*(uint32_t*)buf = regs->per.ocpwp_l3_clkstctrl;
			break;
		case CM_PER_TIMER2_CLKCTRL:
			*(uint32_t*)buf = regs->per.timer2_clkctrl;
			break;
		case CM_DPLL_CLKSEL_TIMER2_CLK:
			*(uint32_t*)buf = regs->dpll.clksel_timer2_clk;
			break;
		default:
			printf("Can not read the register at 0x%x in prcm_am335x\n", offset);
			return Invarg_exp;
	}
	//printf("In %s offset 0x%x buf 0x%x\n", __func__, offset, *(uint32_t*)buf);
	return No_exp;

}

static exception_t prcm_am335x_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct prcm_am335x_device *dev = opaque->obj;
	prcm_am335x_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	//printf("In %s offset 0x%x buf 0x%x\n", __func__, offset, *buf);
	switch(offset) {
		case CM_PER_L4LS_CLKSTCTRL:
			regs->per.l4ls_clkstctrl &= ~0x3;
		       	regs->per.l4ls_clkstctrl |= ((*buf) & 0x3);
			break;
		case CM_PER_L3S_CLKSTCTRL:
			regs->per.l3s_clkstctrl &= ~0x3;
			regs->per.l3s_clkstctrl |= ((*buf) & 0x3);
			break;
		case CM_PER_L3_CLKSTCTRL:
			regs->per.l3_clkstctrl &= ~0x3;
			regs->per.l3_clkstctrl |= ((*buf) & 0x3);
			break;
		case CM_PER_L4LS_CLKCTRL:
			regs->per.l4ls_clkctrl = *buf;
			break;
		case CM_PER_L3_INSTR_CLKCTRL:
			regs->per.l3_instr_clkctrl = *buf;
			break;
		case CM_PER_L3_CLKCTRL:
			regs->per.l3_clkctrl = *buf;
			break;
		case CM_PER_OCPWP_L3_CLKSTCTRL:
			regs->per.ocpwp_l3_clkstctrl &= ~0x3;
			regs->per.ocpwp_l3_clkstctrl |= ((*buf) & 0x3);
			break;
		case CM_PER_TIMER2_CLKCTRL:
			regs->per.timer2_clkctrl = *buf;
			break;
		case CM_DPLL_CLKSEL_TIMER2_CLK:
			regs->dpll.clksel_timer2_clk = *buf;
			break;
		default:
			printf("Can not write the register at 0x%x in prcm_am335x\n", offset);
			return Invarg_exp;
	}
	return No_exp;

}
static conf_object_t* new_prcm_am335x(char* obj_name){
	prcm_am335x_device* dev = skyeye_mm_zero(sizeof(prcm_am335x_device));
	prcm_am335x_reg_t* regs =  skyeye_mm_zero(sizeof(prcm_am335x_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	/* init prcm_am335x regs */
	dev->regs = regs;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = prcm_am335x_read;
	io_memory->write = prcm_am335x_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	
	return dev->obj;
}

exception_t reset_prcm_am335x(conf_object_t* opaque, const char* parameter){
	struct prcm_am335x_device *dev = opaque->obj;
	prcm_am335x_reg_t* regs = dev->regs;
	regs->per.l4ls_clkstctrl = 0xc0102;
	regs->per.l3s_clkstctrl = 0xa;
	regs->per.l3_clkstctrl = 0x12;
	regs->per.ocpwp_l3_clkstctrl = 0x12;
	return No_exp;
}

void free_prcm_am335x(conf_object_t* opaque){
}

void init_prcm_am335x(){
	static skyeye_class_t class_data = {
		.class_name = "prcm_am335x",
		.class_desc = "prcm_am335x",
		.new_instance = new_prcm_am335x,
		.free_instance = free_prcm_am335x,
		.reset_instance = reset_prcm_am335x,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}
