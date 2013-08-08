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
const char* c6k_regstr[] = {
"A0", "A1", "A2","A3","A4","A5","A6","A7",
"A8","A9","A10","A11","A12","A13","A14","A15",
"A16","A17","A18","A19","A20","A21","A22","A23",
"A24","A25","A26","A27","A28","A29","A30","A31",
"B0","B1","B2",	"B3","B4","B5",	"B6","B7",
"B8","B9","B10","B11","B12","B13","B14","B15",
"B16","B17","B18","B19","B20","B21","B22","B23",
"B24","B25","B26","B27","B28","B29","B30","B31",
"PC_REG","CSR_REG","TSR_REG","ILC_REG","RILC_REG",
NULL
};
static char * arch_name = "c6k";
static c6k_core_t* core;
static void per_cpu_step();
static void per_cpu_stop();
extern uint32_t exec_insn(c6k_core_t* core, uint32_t* fetch_packet);
static void register_core_chp(c6k_core_t* core){
	char buf[100];
	int i;
	for(i = 0; i < 32; i++){
		sprintf(buf, "A%d", i);
		add_chp_data((void*)(&core->gpr[GPR_A][i]),sizeof(core->gpr[GPR_A][i]), buf);
	}
	for(i = 0; i < 32; i++){
		sprintf(buf, "B%d", i);
		add_chp_data((void*)(&core->gpr[GPR_B][i]),sizeof(core->gpr[GPR_B][i]), buf);
	}
	strcpy(buf, "ILC");
	add_chp_data((void*)(&core->ilc),sizeof(core->ilc), buf);
	strcpy(buf, "PC");
	add_chp_data((void*)(&core->pc),sizeof(core->pc), buf);
	strcpy(buf, "PFC");
	add_chp_data((void*)(&core->pfc),sizeof(core->pfc), buf);
	strcpy(buf, "PFC_BAK1");
	add_chp_data((void*)(&core->pfc_bak1),sizeof(core->pfc_bak1), buf);
	strcpy(buf, "cycles");
	add_chp_data((void*)(&core->cycles),sizeof(core->cycles), buf);
	strcpy(buf, "insn_num");
	add_chp_data((void*)(&core->insn_num),sizeof(core->insn_num), buf);
	strcpy(buf, "delay_slot");
	add_chp_data((void*)(&core->delay_slot),sizeof(core->delay_slot), buf);
	strcpy(buf, "delay_slot_bak1");
	add_chp_data((void*)(&core->delay_slot_bak1),sizeof(core->delay_slot_bak1), buf);
	strcpy(buf, "header");
	add_chp_data((void*)(&core->header),sizeof(core->header), buf);
	strcpy(buf, "parallel");
	add_chp_data((void*)(&core->parallel),sizeof(core->parallel), buf);
	strcpy(buf, "buffer_pos");
	add_chp_data((void*)(&core->buffer_pos),sizeof(core->buffer_pos), buf);

	strcpy(buf, "sploop_begin");
	add_chp_data((void*)(&core->sploop_begin),sizeof(core->sploop_begin), buf);
	strcpy(buf, "sploop_end");
	add_chp_data((void*)(&core->sploop_end),sizeof(core->sploop_end), buf);
	strcpy(buf, "sploop_flag");
	add_chp_data((void*)(&core->sploop_flag),sizeof(core->sploop_flag), buf);
	for(i = 0; i < 32; i++){
		sprintf(buf, "sploop_buffer[%d]", i);
		add_chp_data((void*)(&core->sploop_buffer[i]),sizeof(core->sploop_buffer[i]), buf);
	}
	strcpy(buf, "sploopw_cond");
	add_chp_data((void*)(&core->sploopw_cond),sizeof(core->sploopw_cond), buf);
	strcpy(buf, "sploopw_flag");
	add_chp_data((void*)(&core->sploopw_flag),sizeof(core->sploopw_flag), buf);
	strcpy(buf, "spmask");
	add_chp_data((void*)(&core->spmask),sizeof(core->spmask), buf);
	strcpy(buf, "spmask_begin");
	add_chp_data((void*)(&core->spmask_begin),sizeof(core->spmask_begin), buf);
	strcpy(buf, "spmask_end");
	add_chp_data((void*)(&core->spmask_end),sizeof(core->spmask_end), buf);
	for(i = 0; i < 64; i++){
		sprintf(buf, "wb_result[%d]", i);
		add_chp_data((void*)(&core->wb_result[i]),sizeof(core->wb_result[i]), buf);
	}
	for(i = 0; i < 64; i++){
		sprintf(buf, "wb_index[%d]", i);
		add_chp_data((void*)(&core->wb_index[i]),sizeof(core->wb_index[i]), buf);
	}

	strcpy(buf, "wb_result_pos");
	add_chp_data((void*)(&core->wb_result_pos),sizeof(core->wb_result_pos), buf);
	strcpy(buf, "wb_flag");
	add_chp_data((void*)(&core->wb_flag),sizeof(core->wb_flag), buf);

}
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

	register_core_chp(core);	
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
cpu_config_t c6k_cpus[] = {
	{"c64x", "c6747", 0xFFFFFFFF, 0xFFFFFFFF, 0},
	NULL
};
static int
c6k_parse_cpu (const char *params[])
{
	int i;
	for (i = 0; i < (sizeof (c6k_cpus) / sizeof (cpu_config_t)); i++) {
		printf("In %s, c6k_cpus[i].cpu_name=%s\n", __FUNCTION__, c6k_cpus[i].cpu_name);
		if (!strncmp
		    (params[0], c6k_cpus[i].cpu_name, MAX_PARAM_NAME)) {

			cpu_config_t *p_c6k_cpu = &c6k_cpus[i];
			SKYEYE_INFO("cpu info: %s, %s, %x, %x, %x \n",
				     p_c6k_cpu->cpu_arch_name,
				     p_c6k_cpu->cpu_name,
				     p_c6k_cpu->cpu_val,
				     p_c6k_cpu->cpu_mask,
				     p_c6k_cpu->cachetype);
			return 0;

		}
	}
	SKYEYE_ERR ("Error: Unknown cpu name \"%s\"\n", params[0]);
	return -1;
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
static uint32 c6k_get_regnum(){
	return C6K_REGNUM;
	//return MAX_REG_NUM;
}
static char* get_regname_by_id(int id){
        return c6k_regstr[id];
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
	c6k_arch.get_regnum = c6k_get_regnum;
	c6k_arch.get_regname_by_id = get_regname_by_id;

	c6k_arch.get_regval_by_id = c6k_get_regval_by_id;
	c6k_arch.set_regval_by_id = c6k_set_register_by_id;
	register_arch (&c6k_arch);
}
