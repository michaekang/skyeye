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
* @file c6k_cpu.h
* @brief the core definition of c6k
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#ifndef __C6K_CPU_H__
#define __C6K_CPU_H__
#include <stdint.h>
#include <skyeye_types.h>
#define SPLOOP_BUFFER_SIZE 32
typedef struct c6k_core{
	/* General Register */
	uint32_t gpr[2][32];

	/* Control Register */
	uint32_t amr;
	uint32_t gsr;
	uint32_t gfpgfr;
	uint32_t icr;
	uint32_t ier;
	uint32_t ifr;
	uint32_t irp;
	uint32_t isr;
	uint32_t nrp;
	uint32_t pce1;
	uint32_t dier;
	uint32_t dnum;
	uint32_t ecr;
	uint32_t efr;
	uint32_t gplya;
	uint32_t gplyb;
	uint32_t ierr;
	uint32_t ilc;
	uint32_t itsr;

	uint32_t pc;
	uint32_t pfc, pfc_bak1;
	uint64_t cycles, insn_num;
	uint32_t delay_slot, delay_slot_bak1;
	uint32_t header;
	uint32_t parallel;

	uint32_t buffer_pos;
	uint32_t sploop_begin, sploop_end;
	uint32_t sploop_flag;
	/* store the instruction */
	uint32_t sploop_buffer[32];
	uint32_t sploopw_cond;
	uint32_t sploopw_flag;
	uint32_t spmask;
	generic_address_t spmask_begin, spmask_end;

	/* save the result of WB */
	uint32_t wb_result[64];
	uint32_t wb_index[64];
	uint32_t wb_result_pos;
	uint32_t wb_flag;
}c6k_core_t;
typedef enum{
	GPR_A = 0,
	GPR_B
}gpr_t;

typedef enum{
	L1_UNIT = 0,
	L2_UNIT,
	S1_UNIT,
	S2_UNIT,
	M1_UNIT,
	M2_UNIT,
	D1_UNIT,
	D2_UNIT
}function_unit_t;
#define FP_SIZE 8
#endif
