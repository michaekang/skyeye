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
* @file skyeye_mach_c6713.c
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

	conf_object_t* conf_obj = get_conf_obj("c6713_mach_space");
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
	conf_object_t* conf_obj = get_conf_obj("c6713_mach_space");
	//printf("In %s, addr=0x%x\n", __FUNCTION__, addr);
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

void c6713_io_reset(void* state){
	return;
}

exception_t set_conf_attr(conf_object_t* obj, char* attr_name, attr_value_t* value);
void c6713_mach_fini(void* state, machine_config_t* mach);
void c6713_mach_init(void* state,  machine_config_t * mach){
	exception_t ret;
	if(!mach)
        {
                SKYEYE_ERR ("Error: No machine config structure\n");
                exit( -1 );
        }
        else
        {
                SKYEYE_INFO("%s(): %s initialized\n", __func__, mach->machine_name);
        }
	attr_value_t* value;
	mach->mach_io_reset = c6713_io_reset;
	mach->mach_fini = c6713_mach_fini;

	addr_space_t* phys_mem = new_addr_space("c6713_mach_space");
	mach->phys_mem = phys_mem;

	conf_object_t* emif = pre_conf_obj("emif_6713_0", "emif_6713");
	if(emif == NULL){
		printf("Can not find device emif_6713\n");
		return;
	}
	memory_space_intf* emif_io_memory = (memory_space_intf*)SKY_get_interface(emif, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x1800000, 0x40000, 0x0, emif_io_memory, 1, 1);

	conf_object_t* timer0 = pre_conf_obj("timer_6713_0", "timer_6713");
	memory_space_intf* timer0_io_memory = (memory_space_intf*)SKY_get_interface(timer0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x1940000, 0x40000, 0x0, timer0_io_memory, 1, 1);

	conf_object_t* timer1 = pre_conf_obj("timer_6713_1", "timer_6713");
	memory_space_intf* timer1_io_memory = (memory_space_intf*)SKY_get_interface(timer1, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x1980000, 0x40000, 0x0, timer1_io_memory, 1, 1);

	conf_object_t* intc = pre_conf_obj("intc_6713_0", "intc_6713");
	memory_space_intf* intc_io_memory = (memory_space_intf*)SKY_get_interface(intc, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x19c0000, 0x200, 0x0, intc_io_memory, 1, 1);

	conf_object_t* pll = pre_conf_obj("pll_6713_0", "pll_6713");
	memory_space_intf* pll_io_memory = (memory_space_intf*)SKY_get_interface(pll, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x01b7c000, 0x1000, 0x0, pll_io_memory, 1, 1);


	conf_object_t* uart_term0 = pre_conf_obj("uart_term_0", "uart_term");
	conf_object_t* uart0 = pre_conf_obj("uart_0", "leon2_uart");
	value = make_new_attr(Val_Object, uart_term0);
	ret = set_conf_attr(uart0, "term", value);

	memory_space_intf* uart0_io_memory = (memory_space_intf*)SKY_get_interface(uart0, MEMORY_SPACE_INTF_NAME);
	ret = add_map(phys_mem, 0x1b7e000, 0x10, 0x0, uart0_io_memory, 1, 1); /* Use reserved address map */
	if(ret != No_exp){
		skyeye_log(Error_log, __FUNCTION__, "Can not register io memory for uart0 controller\n");
	}

	conf_object_t* can_zlg = pre_conf_obj("can_zlg_0", "can_zlg");
	can_ops_intf* can_ops = (can_ops_intf*)SKY_get_interface(can_zlg, CAN_OPS_INTF_NAME);
	conf_object_t* can_sja1000 = pre_conf_obj("can_sja1000_0", "can_sja1000");
	can_ops_intf* sja1000_can_ops = (can_ops_intf*)SKY_get_interface(can_sja1000, CAN_OPS_INTF_NAME);
	sja1000_can_ops->obj = can_zlg;
	sja1000_can_ops->start = can_ops->start;
	sja1000_can_ops->stop = can_ops->stop;
	sja1000_can_ops->transmit = can_ops->transmit;
	sja1000_can_ops->receive = can_ops->receive;

	memory_space_intf* sja1000_io_memory = (memory_space_intf*)SKY_get_interface(can_sja1000, MEMORY_SPACE_INTF_NAME);
        ret = add_map(phys_mem, 0xB0000040, 0x10, 0x0, sja1000_io_memory, 1, 1); /* Use reserved address map */
        if(ret != No_exp){
                skyeye_log(Error_log, __FUNCTION__, "Can not register io memory for sja1000 controller\n");
        }
	
        machine_config_t * this_mach = mach;
        
        this_mach->mach_io_read_byte = io_read_byte;
        this_mach->mach_io_write_byte = io_write_byte;
        this_mach->mach_io_read_halfword = io_read_halfword;
        this_mach->mach_io_write_halfword = io_write_halfword;
        this_mach->mach_io_read_word = io_read_word;
        this_mach->mach_io_write_word = io_write_word;

	return;
}

void c6713_mach_fini(void* state,  machine_config_t * mach){
	conf_object_t* can_zlg = get_conf_obj("can_zlg_0");
	if(can_zlg == NULL)
		; /* maybe freed in other place */
	else
		free_conf_obj(can_zlg);
}
