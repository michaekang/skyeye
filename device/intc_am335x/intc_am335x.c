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
* @file intc_am335x.c
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

#include "intc_am335x.h"

void reset_intc_am335x(conf_object_t* opaque, const char* parameter);

static int intc_am335x_raise_signal(conf_object_t *opaque, int line){
	struct intc_am335x_device *dev = opaque->obj;
	intc_am335x_reg_t* regs = dev->regs;
	uint32_t index = line / REG_SIZE;
	uint32_t reg_off = line % REG_SIZE;
	/* 
	 * The current incoming interrupt status before masking is readable
	 * from the MPU_INTC.INTC_ITRn register.
	 */
	regs->itr[index] |= reg_off;
	/* Judge the interrupt source is unmasked */
	if(!((regs->mir[index] >> reg_off) & 0x1))
		return Not_found_exp;
	/* Judge the priority threshold */
	uint16_t threshold = (regs->ilr[index] >> 1) & 0x3f;
	if(threshold >= regs->threshold)
		return Not_found_exp;

	interrupt_signal_t interrupt_signal;
	/*
	 * update sir_irq or sir_fiq 's bit ACTIVE
	 * before priority sorting is done, the interrupt status is readable
	 * from the PENDING_IRQn and PENDING_FIQn registers.
	 */
	if(regs->ilr[line] & 0x1 == 0x0){
		/* IRQ */
		regs->pending_irq[index] |= (0x1 << reg_off);
		regs->sir_irq |= line;
		interrupt_signal.arm_signal.irq =  High_level;
	}else{
		/* FIQ */
		regs->pending_fiq[index] |= (0x1 << reg_off);
		interrupt_signal.arm_signal.firq =  High_level;
		regs->sir_fiq |= line;
	}
	interrupt_signal.arm_signal.reset =  Prev_level;
	/* We don't need the mechanism of priority sorting temporarily */
	send_signal(&interrupt_signal);

	return No_exp;
}

static int intc_am335x_lower_signal(conf_object_t *opaque, int line){

	return No_exp;
}

static exception_t intc_am335x_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct intc_am335x_device *dev = opaque->obj;
	intc_am335x_reg_t* regs = dev->regs;
	uint32_t index = 0;
	switch(offset) {
		case INTC_REVISION:
			*(uint32_t*)buf = regs->revision;
			break;
		case INTC_SYSCONFIG:
			*(uint32_t*)buf = regs->sysconfig;
			break;
		case INTC_SYSSTATUS:
			*(uint32_t*)buf = regs->sysstatus;
			break;
		case INTC_SIR_IRQ: 
			*(uint32_t*)buf = regs->sir_irq;
			break;
		case INTC_SIR_FIQ:
			*(uint32_t*)buf = regs->sir_fiq;
			break;
		case INTC_CONTROL:
			printf("In %s:%d register is write only\n", __func__, offset);
			break;
		case INTC_PROTECTION:
			*(uint32_t*)buf = regs->protection;
			break;
		case INTC_IDLE: 
			*(uint32_t*)buf = regs->idle;
			break;
		case INTC_IRQ_PRIORITY:
			*(uint32_t*)buf = regs->irq_priority;
			break;
		case INTC_FIQ_PRIORITY:
			*(uint32_t*)buf = regs->fiq_priority;
			break;
		case INTC_THRESHOLD: 
			*(uint32_t*)buf = regs->threshold;
			break;
		case INTC_ITR(0):
		case INTC_ITR(1):
		case INTC_ITR(2):
		case INTC_ITR(3):
			index = (offset - 0x80) / 0x20;
			*(uint32_t*)buf = regs->itr[index];
			break;
		case INTC_MIR(0):
		case INTC_MIR(1):
		case INTC_MIR(2):
		case INTC_MIR(3):
			index = (offset - 0x84) / 0x20;
			*(uint32_t*)buf = regs->mir[index];
			break;
		case INTC_MIR_CLEAR(0):
		case INTC_MIR_CLEAR(1):
		case INTC_MIR_CLEAR(2):
		case INTC_MIR_CLEAR(3):
			/* reads return 0 */ 
			*(uint32_t*)buf = 0;
			break;
		case INTC_MIR_SET(0):
		case INTC_MIR_SET(1):
		case INTC_MIR_SET(2):
		case INTC_MIR_SET(3):
			/* reads return 0 */ 
			*(uint32_t*)buf = 0;
			break;
		case INTC_ISR_CLEAR(0):
		case INTC_ISR_CLEAR(1):
		case INTC_ISR_CLEAR(2):
		case INTC_ISR_CLEAR(3):
			/* reads return 0 */ 
			*(uint32_t*)buf = 0;
			break;
		case INTC_PENDING_IRQ(0):
		case INTC_PENDING_IRQ(1):
		case INTC_PENDING_IRQ(2):
		case INTC_PENDING_IRQ(3):
			index = (offset - 0x98) / 0x20;
			*(uint32_t*)buf = regs->pending_irq[index];
			break;
		case INTC_PENDING_FIQ(0):
		case INTC_PENDING_FIQ(1):
		case INTC_PENDING_FIQ(2):
		case INTC_PENDING_FIQ(3):
			index = (offset - 0x9c) / 0x20;
			*(uint32_t*)buf = regs->pending_fiq[index];
			break;
		default:
			if(offset >= INTC_ILR(0) && offset <= INTC_ILR(127)){
				index = (offset - 0x100) / 0x4;
				*(uint32_t*)buf = regs->ilr[index];
				break;
			}else{

				printf("Can not read the register at 0x%x in intc_am335x\n", offset);
				return Invarg_exp;
			}
	}
	return No_exp;

}

static exception_t intc_am335x_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct intc_am335x_device *dev = opaque->obj;
	intc_am335x_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	uint32_t index = 0;
	switch(offset) {
		case INTC_REVISION:
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		case INTC_SYSCONFIG:
			if((*buf) & 0x2){
				printf("reset the interrupt controler\n");
				reset_intc_am335x(opaque, NULL);
				regs->sysstatus |= 0x1;
			}
			if((*buf) & 0x1)
				printf("Automatic OCP clock gating strategy is applied\n");
			break;
		case INTC_SYSSTATUS:
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		case INTC_SIR_IRQ: 
			regs->sir_irq= *buf;
			break;
		case INTC_SIR_FIQ:
			regs->sir_fiq = *buf;
			break;
		case INTC_CONTROL:
			regs->sir_fiq = *buf;
			break;
		case INTC_PROTECTION:
			regs->protection = *buf;
			break;
		case INTC_IDLE: 
			regs->idle = *buf;
			break;
		case INTC_IRQ_PRIORITY:
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		case INTC_FIQ_PRIORITY:
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		case INTC_THRESHOLD:
			regs->threshold = *buf;
			break;
		case INTC_ITR(0):
		case INTC_ITR(1):
		case INTC_ITR(2):
		case INTC_ITR(3):
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		case INTC_MIR(0):
		case INTC_MIR(1):
		case INTC_MIR(2):
		case INTC_MIR(3):
			index = (offset - 0x84) / 0x20;
			regs->mir[index]= *buf;
			break;
		case INTC_MIR_CLEAR(0):
		case INTC_MIR_CLEAR(1):
		case INTC_MIR_CLEAR(2):
		case INTC_MIR_CLEAR(3):
			index = (offset - 0x88) / 0x20;
			/* Write 1 clears the mask bit to 0 */
			regs->mir[index] &= ~(*buf);
			break;
		case INTC_MIR_SET(0):
		case INTC_MIR_SET(1):
		case INTC_MIR_SET(2):
		case INTC_MIR_SET(3):
			index = (offset - 0x8c) / 0x20;
			/* Write 1 sets the mask bit to 1 */
			regs->mir[index] |= (*buf);
			break;
		case INTC_ISR_SET(0):
		case INTC_ISR_SET(1):
		case INTC_ISR_SET(2):
		case INTC_ISR_SET(3):
			index = (offset - 0x90) / 0x20;
			regs->isr_set[index] = *buf;
			/* Writes 1, soft interrupt */
			printf("In %s gen soft interrupt\n", __func__);
			break;
		case INTC_ISR_CLEAR(0):
		case INTC_ISR_CLEAR(1):
		case INTC_ISR_CLEAR(2):
		case INTC_ISR_CLEAR(3):
			index = (offset - 0x90) / 0x20;
			regs->isr_clear[index] = *buf;
			/* Writes 1, clear soft interrupt */
			printf("In %s clear soft interrupt\n", __func__);
			break;
		case INTC_PENDING_IRQ(0):
		case INTC_PENDING_IRQ(1):
		case INTC_PENDING_IRQ(2):
		case INTC_PENDING_IRQ(3):
		case INTC_PENDING_FIQ(0): 
		case INTC_PENDING_FIQ(1):
		case INTC_PENDING_FIQ(2):
		case INTC_PENDING_FIQ(3):
			printf("In %s:%d register is read only\n", __func__, offset);
			break;
		default:
			if(offset >= INTC_ILR(0) && offset <= INTC_ILR(127)){
				index = (offset - 0x100) / 0x4;
				regs->ilr[index] = *buf;
				break;
			}else{
				printf("Can not write the register at 0x%x in %s\n", offset, __func__);
				return Invarg_exp;
			}
	}
	return No_exp;

}
static conf_object_t* new_intc_am335x(char* obj_name){
	intc_am335x_device* dev = skyeye_mm_zero(sizeof(intc_am335x_device));
	intc_am335x_reg_t* regs =  skyeye_mm_zero(sizeof(intc_am335x_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	/* init intc_am335x regs */
	dev->regs = regs;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = intc_am335x_read;
	io_memory->write = intc_am335x_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);

	general_signal_intf* signal = skyeye_mm_zero(sizeof(general_signal_intf));
	signal->conf_obj = dev->obj;
	signal->raise_signal = intc_am335x_raise_signal;
	signal->lower_signal = intc_am335x_lower_signal;
	SKY_register_interface(signal, obj_name, GENERAL_SIGNAL_INTF_NAME);

	return dev->obj;
}

void reset_intc_am335x(conf_object_t* opaque, const char* parameter)
{
	struct intc_am335x_device *dev = opaque->obj;
	intc_am335x_reg_t* regs = dev->regs;
	regs->revision = 0x50;
	return;
}

void free_intc_am335x(conf_object_t* dev){
	
}

void init_intc_am335x(){
	static skyeye_class_t class_data = {
		.class_name = "intc_am335x",
		.class_desc = "intc_am335x",
		.new_instance = new_intc_am335x,
		.free_instance = free_intc_am335x,
		.reset_instance = reset_intc_am335x,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}
