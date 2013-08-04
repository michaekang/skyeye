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
* @file c6k_arch_interface.c
* @brief The definition of c6k arch
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#ifdef __MINGW32__
#include <windows.h>
#endif

#include <signal.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "skyeye_internal.h"
#include "skyeye_types.h"
#include "skyeye_config.h"
#include "skyeye_options.h"
#include "skyeye_exec.h"
#include "skyeye_cell.h"
#include "skyeye_arch.h"
#include "skyeye_mm.h"
#include "skyeye_obj.h"
#include "c6k_cpu.h"
#include "regformat/c6k_regformat.h"

static char * arch_name = "c6k";
static c6k_core_t* core;
static void per_cpu_step();
static void per_cpu_stop();
extern uint32_t exec_insn(c6k_core_t* core, uint32_t* fetch_packet);
void
c6k_init_state ()
{
	/* FIXME, should check memory allocation failure */
	core = (c6k_core_t*)skyeye_mm_zero(sizeof(c6k_core_t));
	skyeye_exec_t* exec = create_exec();
	exec->priv_data = get_conf_obj_by_cast(core, "c6k_core_t");
	//exec->priv_data = get_conf_obj_by_cast(core, "ARMul_State");
	exec->run = per_cpu_step;
	exec->stop = per_cpu_stop;
	add_to_default_cell(exec);

	/* Set arm endianess is Little endian */
	generic_arch_t *arch_instance = get_arch_instance(NULL);
	arch_instance->endianess = Little_endian;

	core->wb_result_pos = 0;

	core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
	core->spmask_begin = 0xFFFFFFFF;
	core->buffer_pos = 0;
	
	return;
}
void
c6k_reset_state ()
{
}

/* Execute a single instruction.  */
static void
c6k_step_once ()
{
	
	uint32_t addr = core->pc;
	uint32_t fetch_packet[FP_SIZE];
	int status;
	//printf("In %s, pc=0x%x\n", __FUNCTION__, core->pc);
	uint32_t v;
	bus_read(32, 0x817b68, &v);
	core->pce1 = addr & 0xFFFFFFE0;
	if(core->pc == 0x816680){
		//printf("In %s, enter abort , pc=0x%x\n", __FUNCTION__, addr);
		//exit(0);
	}
	int i = 0;
	for(; i < FP_SIZE;i++ ){
		status = bus_read(32, core->pce1 + i * 4, &fetch_packet[i]);
		//printf("In %s, insn=0x%x, read from 0x%x\n", __FUNCTION__, fetch_packet[i], core->pce1 + i * 4);
		/* FIXME, status should be checked */
	}
	if((fetch_packet[FP_SIZE - 1] >> 28) == 0xe){
		core->header = fetch_packet[FP_SIZE - 1];
	}
	else
		core->header = 0x0;
	exec_insn(core, fetch_packet);	
	if(core->pc == 0)
		exit(-1);
	return;
}
static void per_cpu_step()
{
	//printf("In %s, pc=0x%x\n", __FUNCTION__, core->pc);
	c6k_step_once();
	return;
}
static void per_cpu_stop()
{
	return;
}

static void
c6k_set_pc (generic_address_t addr)
{
	core->pc = addr;
	return;
}
static generic_address_t c6k_get_pc(){
	return core->pc;
}
/*
cpu_config_t c6k_cpu[] = {
	{"c6421", "c6421", 0xffffffff, 0xfffffff0, NONCACHE}
	{NULL,NULL,0,0,0}
};
*/
void c6421_io_reset(void* state){
	return;
}
void c6421_mach_init(void* state,  machine_config_t * mach){
	mach->mach_io_reset = c6421_io_reset;
	return;
}
machine_config_t c6k_machines[] = {
	{"c6k", c6421_mach_init, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL},
};

//chy 2006-04-15
static int 
c6k_ICE_write_byte (generic_address_t addr, uint8_t v)
{
	return 0;
}
static int
c6k_ICE_read_byte(generic_address_t addr, uint8_t * pv){
	return 0;
}
static int
c6k_parse_cpu (cpu_config_t * cpu, const char *param[])
{
	return 0;
}

static int 
c6k_parse_mem(int num_params, const char* params[])
{
#if 0
	char name[MAX_PARAM_NAME], value[MAX_PARAM_NAME];
	int i, num;
	c6k_mem_config_t *mc = &c6k_mem_config;
	c6k_mem_bank_t *mb = mc->mem_banks;

	mc->bank_num = mc->current_num++;

	num = mc->current_num - 1;	/*mem_banks should begin from 0. */
	mb[num].filename[0] = '\0';
	for (i = 0; i < num_params; i++) {
		if (split_param (params[i], name, value) < 0)
			SKYEYE_ERR
				("Error: mem_bank %d has wrong parameter \"%s\".\n",
				 num, name);

		if (!strncmp ("map", name, strlen (name))) {
			if (!strncmp ("M", value, strlen (value))) {
				mb[num].read_byte = c6k_real_read_byte;
				mb[num].write_byte = c6k_real_write_byte;
				mb[num].read_halfword = c6k_real_read_halfword;
				mb[num].write_halfword = c6k_real_write_halfword;
				mb[num].read_word = c6k_real_read_word;
				mb[num].write_word = c6k_real_write_word;
				mb[num].read_doubleword = c6k_real_read_doubleword;
				mb[num].write_doubleword = c6k_real_write_doubleword;
				mb[num].type = MEMTYPE_RAM;
			}
			else if (!strncmp ("I", value, strlen (value))) {
				mb[num].read_byte = c6k_io_read_byte;
				mb[num].write_byte = c6k_io_write_byte;
				mb[num].read_halfword = c6k_io_read_halfword;
				mb[num].write_halfword = c6k_io_write_halfword;
				mb[num].read_word = c6k_io_read_word;
				mb[num].write_word = c6k_io_write_word;
				mb[num].read_doubleword = c6k_io_read_doubleword;
				mb[num].write_doubleword = c6k_io_write_doubleword;

				mb[num].type = MEMTYPE_IO;

				/*ywc 2005-03-30 */
			}
			else if (!strncmp ("F", value, strlen (value))) {
				mb[num].read_byte = c6k_flash_read_byte;
				mb[num].write_byte = c6k_flash_write_byte;
				mb[num].read_halfword = c6k_flash_read_halfword;
				mb[num].write_halfword = c6k_flash_write_halfword;
				mb[num].read_word = c6k_flash_read_word;
				mb[num].write_word = c6k_flash_write_word;
				mb[num].read_doubleword = c6k_flash_read_doubleword;
				mb[num].write_doubleword = c6k_flash_write_doubleword;
				mb[num].type = MEMTYPE_FLASH;

			}
			else {
				SKYEYE_ERR
					("Error: mem_bank %d \"%s\" parameter has wrong value \"%s\"\n",
					 num, name, value);
			}
		}
		else if (!strncmp ("type", name, strlen (name))) {
			//chy 2003-09-21: process type
			if (!strncmp ("R", value, strlen (value))) {
				if (mb[num].type == MEMTYPE_RAM)
					mb[num].type = MEMTYPE_ROM;
				mb[num].write_byte = c6k_warn_write_byte;
				mb[num].write_halfword = c6k_warn_write_halfword;
				mb[num].write_word = c6k_warn_write_word;
			}
		}
		else if (!strncmp ("addr", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb[num].addr = strtoul (value, NULL, 16);
			else
				mb[num].addr = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("size", name, strlen (name))) {

			if (value[0] == '0' && value[1] == 'x')
				mb[num].len = strtoul (value, NULL, 16);
			else
				mb[num].len = strtoul (value, NULL, 10);

		}
		else if (!strncmp ("file", name, strlen (name))) {
			strncpy (mb[num].filename, value, strlen (value) + 1);
		}
		else if (!strncmp ("boot", name, strlen (name))) {
			/*this must be the last parameter. */
			if (!strncmp ("yes", value, strlen (value)))
				skyeye_config.start_address = mb[num].addr;
		}
		else {
			SKYEYE_ERR
				("Error: mem_bank %d has unknow parameter \"%s\".\n",
				 num, name);
		}
	}
#endif
	return 0;
}
static exception_t c6k_set_register_by_id(int id, uint32 value){
	if(id >= A0 && id <= A31)
		core->gpr[GPR_A][id - A0];
	if(id > A31 && id <= B31)
		core->gpr[GPR_B][id - B0] = value;
	else if(id == PC_REG)
		core->pc;
		
	return No_exp;
}

static uint32 c6k_get_regval_by_id(int id){
	if (id == PC_REG)
		return core->pc;
	else if(id >= A0 && id <= A31)
		return core->gpr[GPR_A][id - A0];
	else if(id > A31 && id <= B31)
		return core->gpr[GPR_B][id - B0];
	else
		return 0xFFFFFFFF; /* something wrong */
}

void
init_c6k_arch ()
{
	static arch_config_t c6k_arch;
	memset(&c6k_arch, '\0', sizeof(c6k_arch));

	c6k_arch.arch_name = arch_name;
	c6k_arch.init = c6k_init_state;
	c6k_arch.reset = c6k_reset_state;
	c6k_arch.step_once = c6k_step_once;
	c6k_arch.set_pc = c6k_set_pc;
	c6k_arch.get_pc = c6k_get_pc;
	c6k_arch.ICE_write_byte = c6k_ICE_write_byte;
	c6k_arch.ICE_read_byte = c6k_ICE_read_byte;
	c6k_arch.parse_cpu = c6k_parse_cpu;
	c6k_arch.parse_mem = c6k_parse_mem;

	c6k_arch.get_regval_by_id = c6k_get_regval_by_id;
	c6k_arch.set_regval_by_id = c6k_set_register_by_id;
	register_arch (&c6k_arch);
}
