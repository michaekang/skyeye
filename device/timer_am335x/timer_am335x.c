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
* @file timer_am335x.c
* @brief The implementation of system controller
* @author Kewei Yu: keweihk@gmail.com
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

#include "timer_am335x.h"

void reset_timer_am335x(conf_object_t *opaque, const char* parameters);

static exception_t timer_am335x_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct timer_am335x_device *dev = opaque->obj;
	timer_am335x_reg_t* regs = dev->regs;
	switch(offset) {
		case DMTIMER_TIDR:
			*(uint32_t*)buf = regs->tidr;
			break;
		case DMTIMER_TIOCP_CFG:
			*(uint32_t*)buf = regs->tiocp_cfg;
			break;
		case DMTIMER_TCLR:
			*(uint32_t*)buf = regs->tclr;
			break;
		case DMTIMER_TCRR:
			*(uint32_t*)buf = regs->tcrr;
			break;
		case DMTIMER_TLDR:
			*(uint32_t*)buf = regs->tldr;
			break;
		case DMTIMER_TTGR:
			*(uint32_t*)buf = regs->ttgr;
			break;

		default:
			printf("Can not read the register at 0x%x in timer_am335x\n", offset);
			return Invarg_exp;
	}
	return No_exp;

}

static exception_t timer_am335x_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct timer_am335x_device *dev = opaque->obj;
	timer_am335x_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	switch(offset) {
		case DMTIMER_TIDR:
			regs->tidr = *buf;
			break;
		case DMTIMER_TIOCP_CFG:
			regs->tiocp_cfg = *buf;
			if((*buf) & DMTIMER_TIOCP_CFG_SOFTRESET == DMTIMER_TIOCP_CFG_SOFTRESET_INITIATE){
				regs->tiocp_cfg &= ~DMTIMER_TIOCP_CFG_SOFTRESET_ONGOING;
				/* initiate software reset */
				reset_timer_am335x(opaque, NULL);

				regs->tiocp_cfg |=DMTIMER_TIOCP_CFG_SOFTRESET_DONE;
			}
			if(((*buf) & DMTIMER_TIOCP_CFG_EMUFREE) >> DMTIMER_TIOCP_CFG_EMUFREE_SHIFT
				       	== DMTIMER_TIOCP_CFG_EMUFREE_TIMER_FREE){
				/* The timer runs free, regardless of PINSUSPENDN value */ 
			}else{
				/*  The timer is frozen in emulation mode */
			}
			uint8_t idlemode = ((*buf) & DMTIMER_TIOCP_CFG_IDLEMODE)
			       	>> DMTIMER_TIOCP_CFG_IDLEMODE_SHIFT;
			if(idlemode == DMTIMER_TIOCP_CFG_IDLEMODE_NOIDLE){
				/* No-idle mode */
			}else if(idlemode == DMTIMER_TIOCP_CFG_IDLEMODE_SMART){
				/* Smart-idle mode */
			}else if(idlemode == DMTIMER_TIOCP_CFG_IDLEMODE_WAKEUP){
				/* Smart-idle wakeup-capable mode */ 
			}else{
				/* Force-idle mode */
			}
			break;
		case DMTIMER_TCLR:
			regs->tclr = *buf;
			if(((*buf) & DMTIMER_TCLR_ST) >> DMTIMER_TCLR_ST_SHIFT == DMTIMER_TCLR_ST_START){
				/* Start timer */
			}else{
				/* Stop timeOnly the counter is frozen */
			}
			if(((*buf) & DMTIMER_TCLR_AR) >> DMTIMER_TCLR_AR_SHIFT == DMTIMER_TCLR_AR_AUTO){
				/* Auto-reload timer */
			}else{
				/* One shot timer */
			}
			if(((*buf) & DMTIMER_TCLR_CE) >> DMTIMER_TCLR_CE_SHIFT == DMTIMER_TCLR_CE_ENABLE){
				/* Compare mode is enabled */
			}else{
				/* Compare mode is disabled */
			}
			break;
		case DMTIMER_TCRR:
			regs->tclr = *buf;
			break;
		case DMTIMER_TLDR:
			regs->tldr = *buf;
			break;
		case DMTIMER_TTGR:
			regs->tcrr = regs->tldr;
			break;
		case DMTIMER_TMAR:
			regs->tldr = *buf;
			break;
		default:
			printf("Can not write the register at 0x%x in timer_am335x\n", offset);
			return Invarg_exp;
	}
	return No_exp;

}
static conf_object_t* new_timer_am335x(char* obj_name){
	timer_am335x_device* dev = skyeye_mm_zero(sizeof(timer_am335x_device));
	timer_am335x_reg_t* regs =  skyeye_mm_zero(sizeof(timer_am335x_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	/* init timer_am335x regs */
	dev->regs = regs;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = timer_am335x_read;
	io_memory->write = timer_am335x_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	
	return dev->obj;
}

void reset_timer_am335x(conf_object_t *opaque, const char* parameters)
{
	struct timer_am335x_device *dev = opaque->obj;
	timer_am335x_reg_t* regs = dev->regs;
	regs->tidr = 0x40000100;
}

void free_timer_am335x(conf_object_t* dev){
	
}

void init_timer_am335x(){
	static skyeye_class_t class_data = {
		.class_name = "timer_am335x",
		.class_desc = "timer_am335x",
		.new_instance = new_timer_am335x,
		.free_instance = free_timer_am335x,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}
