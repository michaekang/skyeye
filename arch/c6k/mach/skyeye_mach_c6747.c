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
* @file skyeye_mach_c6747.c
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-06
*/

#include <skyeye_config.h>
#include <skyeye_arch.h>
#include <skyeye_sched.h>
#include <skyeye_lock.h>
#include <skyeye_class.h>
#include <skyeye_addr_space.h>
#include "skyeye_internal.h"
#include <skyeye_interface.h>
#include <skyeye_can_ops.h>
#include <skyeye_io.h>
#include <skyeye_log.h>
#include <skyeye_uart.h>
#include <skyeye_mm.h>

static uint32
io_read_word (void *arch_instance, uint32 addr){
	uint32 data = 0;
	int i;

	conf_object_t* conf_obj = get_conf_obj("s3c6410_mach_space");
	addr_space_t* phys_mem = (addr_space_t*)conf_obj->obj;
	exception_t ret = phys_mem->memory_space->read(conf_obj, addr, &data, 4);
	/* Read the data successfully */
	if(ret == No_exp){
		return data;
	}
	return 0;
}
static uint32
io_read_halfword (void *arch_instance, uint32 addr)
{
	return io_read_word (arch_instance, addr);
}

static uint32
io_read_byte (void *arch_instance, uint32 addr){
	return io_read_word (arch_instance, addr);
}
static void
io_write_word (generic_arch_t *state, uint32 addr, uint32 data)
{
	conf_object_t* conf_obj = get_conf_obj("s3c6410_mach_space");
	addr_space_t* phys_mem = (addr_space_t*)conf_obj->obj;
	exception_t ret = phys_mem->memory_space->write(conf_obj, addr, &data, 4);
	/* Read the data successfully */
	if(ret == No_exp){
		return;
	}
	return;
}
static void
io_write_byte (generic_arch_t *state, uint32 addr, uint32 data)
{
	io_write_word (state, addr, data);
	return;
}

static void
io_write_halfword (generic_arch_t *state, uint32 addr, uint32 data)
{
	io_write_word (state, addr, data);
	return;
}

void c6747_io_reset(void* state){
	return;
}
void c6747_mach_init(void* state,  machine_config_t * mach){
	exception_t ret;
	mach->mach_io_reset = c6747_io_reset;

	addr_space_t* phys_mem = new_addr_space("c6747_mach_space");
	conf_object_t* hecc = pre_conf_obj("am35x_hecc_0", "am35x_hecc");
       	ret = add_map(phys_mem, 0x7f008000, 0x1000, 0x0, hecc, 1, 1);

	conf_object_t* can_zlg = pre_conf_obj("can_zlg_0", "can_zlg");

	conf_object_t* uart = pre_conf_obj("leon2_uart_0", "leon2_uart");
       	ret = add_map(phys_mem, 0x7f00a000, 0x1000, 0x0, uart, 1, 1);

	can_ops_intf* can_ops = (can_ops_intf*)SKY_get_interface(can_zlg, CAN_OPS_INTF_NAME);
	can_ops_intf* hecc_can_ops = (can_ops_intf*)SKY_get_interface(hecc, CAN_OPS_INTF_NAME);
	hecc_can_ops->obj = can_zlg;
	hecc_can_ops->start = can_ops->start;
	hecc_can_ops->stop = can_ops->stop;
	hecc_can_ops->transmit = can_ops->transmit;
	hecc_can_ops->receive = can_ops->receive;

	return;
}

