/*
 * skyeye_mach_omap4460x.c - define machine omap4460 for skyeye
 *
 * Copyright (C) 2012 Oubang Shen <shenoubang@gmail.com>
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

#include "omap4460x.h"

omap4460x_io_t omap4460x_io;
#define io omap4460x_io

static void omap4460x_io_do_cycle(generic_arch_t* state) {
	// TODO: function body
	//SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_io_do_cycle not realize\n", __func__, __LINE__);
}

static void omap4460x_io_reset(generic_arch_t* arch_instance) {
	arch_instance->set_regval_by_id(1, 2160);
	// TODO: function body
	SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_io_reset not realize\n",  __func__, __LINE__);
}

static uint32 omap4460x_io_read_word(void* arch_instance, uint32 addr) {
	uint32 data = 0;

	// TODO: read word, how to do?
	conf_object_t* conf_obj = get_conf_obj("omap4460_mach_space");
    addr_space_t* phys_mem = (addr_space_t*)conf_obj->obj;
    exception_t ret = phys_mem->memory_space->read(conf_obj, addr, &data, 4);
    /* Read the data successfully */
    if(ret == No_exp){
        return data;
    }

	switch (addr) {
		case CONTROL_ID_CODE:
			data = 0x0B94E02F;
			break;
		case SCU_CONFIG:
			/* 2 cpus, SMP adn 32KB cache*/
			data = 0x0531;
			break;
		default:
			SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_io_read_word not realize, addr = 0x%x\n",
			__func__, __LINE__, addr);
			break;
	}

	return data;
}

static uint32 omap4460x_io_read_byte(void* arch_instance, uint32 addr) {
	return omap4460x_io_read_word(arch_instance, addr);
}

static uint32 omap4460x_io_read_halfword(void* arch_instance, uint32 addr) {
	return omap4460x_io_read_word(arch_instance, addr);
}

static void omap4460x_io_write_word(generic_arch_t* state, uint32 addr, uint32 data) {
	// TODO: write word how to do?
	SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_io_write_word not realize\n", __func__, __LINE__);
}

static void omap4460x_io_write_byte(generic_arch_t* state, uint32 addr, uint32 data) {
	omap4460x_io_write_word(state, addr, data);
}

static void omap4460x_io_write_halfword(generic_arch_t* state, uint32 addr, uint32 data) {
	omap4460x_io_write_word(state, addr, data);
}

static void omap4460x_update_int(generic_arch_t* state) {
	// TODO: function body
	SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_update_int not realize\n", __func__, __LINE__);
}

static void omap4460x_set_ext_intr(uint32 interrupt) {
	// TODO: function body
	SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_set_intr not realize\n", __func__, __LINE__);
}

static void omap4460x_pending_ext_intr(uint32 interrupt) {
	// TODO: function body
	SKYEYE_LOG_IN_CLR(RED, "In %s, line = %d, omap4460x_pending_intr not realize\n", __func__, __LINE__);
}

static void omap4460x_update_intr(void* mach) {
	generic_arch_t* arch_instance = get_arch_instance("");
	omap4460x_update_int(arch_instance);
}

void omap4460x_mach_init(void *arch_instance, machine_config_t *this_mach) {
	this_mach->mach_io_do_cycle = omap4460x_io_do_cycle;
	this_mach->mach_io_reset = omap4460x_io_reset;
	this_mach->mach_io_read_byte = omap4460x_io_read_byte;
	this_mach->mach_io_read_halfword = omap4460x_io_read_halfword;
	this_mach->mach_io_read_word = omap4460x_io_read_word;
	this_mach->mach_io_write_byte = omap4460x_io_write_byte;
	this_mach->mach_io_write_halfword = omap4460x_io_write_halfword;
	this_mach->mach_io_write_word = omap4460x_io_write_word;
	this_mach->mach_update_int = omap4460x_update_int;
	this_mach->mach_set_intr = omap4460x_set_ext_intr;
	this_mach->mach_pending_intr = omap4460x_pending_ext_intr;
	this_mach->mach_update_intr = omap4460x_update_intr;
	this_mach->state = (void*)arch_instance;

	add_chp_data(&omap4460x_io, sizeof(omap4460x_io_t), "omap4460io");
	/* The whole address space */
	addr_space_t* phys_mem = new_addr_space("omap4460_mach_space");
	SKYEYE_DBG("In %s, line = %d, omap4460x_mach_init ok!!!!\n", __func__, __LINE__);
}
