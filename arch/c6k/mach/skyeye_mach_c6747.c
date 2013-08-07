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

	conf_object_t* conf_obj = get_conf_obj("c6747_mach_space");
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
	conf_object_t* conf_obj = get_conf_obj("c6747_mach_space");
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

exception_t set_conf_attr(conf_object_t* obj, char* attr_name, attr_value_t* value);

void c6747_mach_init(void* state,  machine_config_t * mach){
	exception_t ret;
	attr_value_t* value;
	mach->mach_io_reset = c6747_io_reset;

	addr_space_t* phys_mem = new_addr_space("c6747_mach_space");
	conf_object_t* hecc = pre_conf_obj("am35x_hecc_0", "am35x_hecc");
	ret = add_map(phys_mem, 0x7f008000, 0x1000, 0x0, hecc, 1, 1);

	conf_object_t* can_zlg = pre_conf_obj("can_zlg_0", "can_zlg");

	conf_object_t* uart_term0 = pre_conf_obj("uart_term_0", "uart_term");
	conf_object_t* uart0 = pre_conf_obj("uart_0", "leon2_uart");
	value = make_new_attr(Val_Object, uart_term0);
	ret = set_conf_attr(uart0, "term", value);

	memory_space_intf* uart0_io_memory = (memory_space_intf*)SKY_get_interface(uart0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x80000070, 0x10, 0x0, uart0_io_memory, 1, 1);
	if(ret != No_exp){
		skyeye_log(Error_log, __FUNCTION__, "Can not register io memory for uart0 controller\n");
	}

	can_ops_intf* can_ops = (can_ops_intf*)SKY_get_interface(can_zlg, CAN_OPS_INTF_NAME);
	can_ops_intf* hecc_can_ops = (can_ops_intf*)SKY_get_interface(hecc, CAN_OPS_INTF_NAME);
	hecc_can_ops->obj = can_zlg;
	hecc_can_ops->start = can_ops->start;
	hecc_can_ops->stop = can_ops->stop;
	hecc_can_ops->transmit = can_ops->transmit;
	hecc_can_ops->receive = can_ops->receive;
	
        machine_config_t * this_mach = mach;
        if( !this_mach )
        {
                SKYEYE_ERR ("Error: No machine config structure\n");
                exit( -1 );
        }
        else
        {
                SKYEYE_INFO("%s(): %s initialized\n", __func__, mach->machine_name);
        }
        this_mach->mach_io_read_byte = io_read_byte;
        this_mach->mach_io_write_byte = io_write_byte;
        this_mach->mach_io_read_halfword = io_read_halfword;
        this_mach->mach_io_write_halfword = io_write_halfword;
        this_mach->mach_io_read_word = io_read_word;
        this_mach->mach_io_write_word = io_write_word;

	return;
}

