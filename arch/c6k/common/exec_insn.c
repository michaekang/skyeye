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
* @file exec_insn.c
* @brief The instruction interpreter
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#include "c6k_cpu.h"
#include "c6k_decode.h"
#include <stdio.h>
#include <stdlib.h>
#include "skyeye_types.h"
#include "skyeye_bus.h"
#include "skyeye_uart_ops.h"

//#define DEBUG
#include <skyeye_log.h>

int exec_32b_insn(c6k_core_t* core, uint32_t insn);
int decode_instr(uint32_t insn, int32_t *idx, ISEITEM* table, int table_len);
extern int exec_16b_insn(c6k_core_t* core, uint32_t insn);
#define NOT_IMP DBG("In %s:%d, not implement at 0x%x\n", __FUNCTION__, __LINE__, core->pc);exit(-1)
#define PR_ALL_REG 0
void print_all_gpr(c6k_core_t* core){
	int i;
#if PR_ALL_REG 
	DBG("------------------- pc =0x%x(insn_num=%d)-------------------\n", core->pc, core->insn_num);
	for(i = 0; i < 32; i++){
		skyeye_printf_in_color(LIGHT_BLUE, "A[%d]=0x%x\t", i, core->gpr[GPR_A][i]);
		//DBG("A[%d]=0x%x\t", i, core->gpr[GPR_A][i]);
		if(( (i + 1) % 8) == 0)
			DBG("\n");
	}
	for(i = 0; i < 32; i++){
		skyeye_printf_in_color(LIGHT_BLUE, "B[%d]=0x%x\t", i, core->gpr[GPR_B][i]);
		if(( (i + 1) % 8) == 0)
			DBG("\n");
	}
	DBG("delay_slot=0x%x, pfc=0x%x, parallel=%d\n", core->delay_slot, core->pfc, core->parallel);
	DBG("================================================\n", core->pc);
#endif
	return;
}

static inline int calc_a_index(int creg){
	int a;
	if((creg & 0x3) == 0)
		a = 1;
	else if((creg & 0x3) == 1)
		a = 2;
	else if((creg & 0x3) == 2)
		a = 0;
	else /* something wrong */
		;
	return a;
}

static inline int calc_cond(c6k_core_t* core, uint32_t insn){
	int cond = insn >> 28;
	int ret;
	int creg = cond >> 1;
	int z = cond & 0x1;
	DBG("\n\nIn %s, creg=0x%x, z=0x%x pc=0x%x, delay_slot=0x%x, sploop_begin=0x%x\n", __FUNCTION__, creg, z, core->pc, core->delay_slot, core->sploop_begin);
	if(cond == 0)
		return 1;
	else{
		//DBG("In %s, creg=0x%x, z=0x%x pc=0x%x\n", __FUNCTION__, creg, z, core->pc);
		if(z){
			if(creg & 0x4)
				ret = core->gpr[GPR_A][calc_a_index(creg)] ? 0 : 1;
			else{
				ret = core->gpr[GPR_B][creg - 1] ? 0 : 1;
			}
		}
		else{
			if(creg & 0x4)
				ret = core->gpr[GPR_A][calc_a_index(creg)] ? 1 : 0;
			else{
				ret = core->gpr[GPR_B][creg - 1] ? 1 : 0;
			}

		}
	}
	return ret;
}

void write_buffer(c6k_core_t* core, int regno, uint32_t result){
	/* write the result to WB buffer */
	int pos = core->wb_result_pos;
	//core->wb_index[pos] = dst + s * 32;
	core->wb_index[pos] = regno;
	core->wb_result[pos] = result;
	core->wb_result_pos++;
	core->wb_flag = 1;
	DBG("In %s, record wb index=%d, result=0x%x, pos=0x%x\n", __FUNCTION__, core->wb_index[pos], result, pos);
	return;
}

void write_back(c6k_core_t* core){
	int i = 0;
	if(core->wb_flag){
		for(; i < core->wb_result_pos; i++){
			int pos = core->wb_index[i];
			if(pos > 31)
				core->gpr[GPR_B][pos - 32] = core->wb_result[i];
			else
				core->gpr[GPR_A][pos] = core->wb_result[i];
			DBG("WriteBack i=%d, gpr=%d, result=0x%x\n", i, pos, core->wb_result[i]);
		}
		/* end of parallel, we should write back result */
		core->wb_result_pos = 0;
		core->wb_flag = 0;
	}
	return;
}

static void inline dec_delay_slot(c6k_core_t* core){
	if(core->delay_slot){
		core->delay_slot--;
	}
}
static void inline record_sploop_buffer(c6k_core_t* core, uint32_t insn, uint32_t compact_flag){
	if(core->spmask && (core->pc > core->spmask_begin && core->pc <= (core->spmask_end))){
		//skyeye_printf_in_color(RED, "mask instruction 0x%x, pc=0x%x\n", insn, core->pc);
	}
	else{
		if((!core->spmask) || core->pc > core->spmask_end){
			int pos = core->buffer_pos;
			if(compact_flag){
				if(((core->pc - 2) & 0x2) == 0)
					core->sploop_buffer[pos] = insn & 0xFFFF;
				else
					core->sploop_buffer[pos] = insn >> 16;
			}
			else{
				core->sploop_buffer[pos] = insn;
			}
			core->buffer_pos++;
			//skyeye_printf_in_color(PURPLE, "In %s, record instr at sploop_buffer %d, instr=0x%x\n", __FUNCTION__, core->buffer_pos, core->sploop_buffer[pos]);
			//skyeye_printf_in_color(PURPLE, "In %s, pc=0x%x, spmask_begin=0x%x, spmask_end=0x%x, record instr at sploop_buffer\n", __FUNCTION__, core->pc, core->spmask_begin, core->spmask_end);
			core->spmask = 0; /* end of spmask */
		}
	}
}
static void inline set_delay_slot(c6k_core_t* core){
}
#define WORD_ALIGN(a, b) (a << b)
static inline generic_address_t  calc_addr(c6k_core_t* core, int base, int offset, int mode, int y, int align){
	generic_address_t addr;
	//int align = 2;
	switch(mode){
		case 0:
			addr = core->gpr[y][base] - WORD_ALIGN(offset, align);
			break;
		case 1:
			addr = core->gpr[y][base] + WORD_ALIGN(offset, align);
			break;
		case 4:
			addr = core->gpr[y][base] - WORD_ALIGN(core->gpr[y][offset], align);
			break;
		case 5:
			addr = core->gpr[y][base] + WORD_ALIGN(core->gpr[y][offset], align);
			break;
		case 8:
			core->gpr[y][base] -= WORD_ALIGN(offset, align);
			addr = core->gpr[y][base];
			break;
		case 9:
			core->gpr[y][base] += WORD_ALIGN(offset, align);
			addr = core->gpr[y][base];
			break;
		case 0xa:
			addr = core->gpr[y][base];
			core->gpr[y][base] -= WORD_ALIGN(offset, align);
			break;
		case 0xb:
			addr = core->gpr[y][base];
			core->gpr[y][base] += WORD_ALIGN(offset, align);
			break;
		case 0xc:
			core->gpr[y][base] -= WORD_ALIGN(core->gpr[y][offset], align);
			addr = core->gpr[y][base];
			break;
		case 0xd:
			core->gpr[y][base] += WORD_ALIGN(core->gpr[y][offset], align);
			addr = core->gpr[y][base];
			break;
		case 0xe:
			addr = core->gpr[y][base];
			core->gpr[y][base] -= WORD_ALIGN(core->gpr[y][offset], align);
			break;
		case 0xf:
			addr = core->gpr[y][base];
			core->gpr[y][base] += WORD_ALIGN(core->gpr[y][offset], align);
			break;
		default:
			NOT_IMP;
			addr = 0xFFFFFFFF;
			break;
	}
	return addr;
}

static int exec_abs(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_abs2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_add_l(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x2){
			int scst5 = SIGN_EXTEND(src1, 5);
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] + scst5;
			else
				core->gpr[s][dst] = core->gpr[s][src2] + scst5;
			DBG("In %s, op=0x%x, insn=0x%x\n", __FUNCTION__, op, insn);
		}
		else if(op == 0x3){
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] + core->gpr[s][src1];
			else
				core->gpr[s][dst] = core->gpr[s][src2] + core->gpr[s][src1];
			DBG("In %s, op=0x%x, insn=0x%x, src1=%d, src2=%d\n", __FUNCTION__, op, insn, src1, src2);

		}
		else{
			DBG("In %s, op=0x%x, insn=0x%x\n", __FUNCTION__, op, insn);
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;

}
static int exec_add_s(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x6){
			int scst5 = SIGN_EXTEND(src1, 5);
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] + scst5;
			else
				core->gpr[s][dst] = core->gpr[s][src2] + scst5;
			DBG("In %s, op=0x%x, insn=0x%x, scst5=%d\n", __FUNCTION__, op, insn, scst5);
		}
		else{
			DBG("In %s, op=0x%x, insn=0x%x\n", __FUNCTION__, op, insn);
			NOT_IMP;
		}

	}
	core->pc += 4;
	return 0;

}
static int exec_add_d(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int op = BITS(7, 12);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);

		if(op == 0x12){
			core->gpr[s][dst] = core->gpr[s][src2] + src1;
			DBG("In %s, op=0x%x, insn=0x%x, scst5=%d\n", __FUNCTION__, op, insn, src1);
		}
		else{
			DBG("In %s, op=0x%x, insn=0x%x\n", __FUNCTION__, op, insn);
			NOT_IMP;
		}

	}
	core->pc += 4;
	return 0;

}

static int exec_addab(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_addad(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int op = BITS(7, 12);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		if(op == 0x3c){
			core->gpr[s][dst] = core->gpr[s][src2] + (core->gpr[s][src1] << 3);
		}
		else{
			NOT_IMP;
		}
		//DST(insn)
	}
	core->pc += 4;
	return 0;
}
static int exec_addah(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_addaw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_addk(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int scst = BITS(7, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		scst = SIGN_EXTEND(scst, 16);
		core->gpr[s][dst] += scst;
		DBG("In %s, dst=%d, scst=%d\n", __FUNCTION__, dst, scst);
	}
	core->pc += 4;
	return 0;
}
static int exec_addkpc(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int scst7 = BITS(16, 22);
		int ucst3 = BITS(13, 15);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		DBG("In %s, scst7=%d, core->pc=0x%x\n", __FUNCTION__, scst7, core->pc);
		if(scst7 & 0x40){
			scst7 = 0xFFFFFF80 | scst7;
		}
		core->gpr[s][dst] = core->pce1 + (scst7 << 2);
		/* Add nop specified by ucst3 */
		DBG("In %s, before ucst3=%d, delay_slot=%d\n", __FUNCTION__, ucst3, core->delay_slot);
		if(core->delay_slot >= ucst3)
			core->delay_slot -= ucst3;
		else
			core->delay_slot = 0;
		DBG("In %s, after ucst3=%d, delay_slot=%d\n", __FUNCTION__, ucst3, core->delay_slot);
	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_addsub(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_addsub2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_addu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_add2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_add4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_and_l(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x7b){
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] & core->gpr[s][src1];
			else
				core->gpr[s][dst] = core->gpr[s][src2] & core->gpr[s][src1];
			DBG("In %s, op=0x%x, insn=0x%x, src1=%d, src2=%d\n", __FUNCTION__, op, insn, src1, src2);

		}
		else if(op == 0x7a){
			int scst5 = SIGN_EXTEND(src1, 5);
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] & scst5;
			else
				core->gpr[s][dst] = core->gpr[s][src2] & scst5;
			DBG("In %s, op=0x%x, insn=0x%x, src1=0x%x, scst5=0x%x src2=%d\n", __FUNCTION__, op, insn, src1, scst5, src2);

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_and_s(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x1f){
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] & core->gpr[s][src1];
			else
				core->gpr[s][dst] = core->gpr[s][src2] & core->gpr[s][src1];
			DBG("In %s, op=0x%x, insn=0x%x, src1=%d, src2=%d\n", __FUNCTION__, op, insn, src1, src2);

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_and_s_cst(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		//DST(insn)
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		DBG("In %s, src1 = 0x%x\n", __FUNCTION__, src1);
		if(src1 & 0x10)
			src1 = 0xFFFFFFE0 | src1;
		core->gpr[s][dst] = core->gpr[s][src2] & src1;
		if(x)
			core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] & src1;
		else
			core->gpr[s][dst] = core->gpr[s][src2] & src1;

	}
	core->pc += 4;
	return 0;
}

static int exec_and_d(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int op = BITS(6, 9);
		int s = BITS(1, 1);
		DBG("In %s, src1 = 0x%x\n", __FUNCTION__, src1);
		if(op == 0x7){
			if(src1 & 0x10)
				src1 = 0xFFFFFFE0 | src1;
			core->gpr[s][dst] = core->gpr[s][src2] & src1;
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	//NOT_IMP;
	return 0;
}

static int exec_andn(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(x)
			core->gpr[s][dst] = core->gpr[s][src1] & (~core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = core->gpr[s][src1] & (~core->gpr[s][src2]);
		DBG("In %s, src1=%d, src2=%d, ret=0x%x\n", __FUNCTION__, src1, src2, core->gpr[s][dst]);
		DBG("In %s, core->gpr[s][src1]=0x%x, ~core->gpr[s][src2]=0x%x, ret=0x%x\n", __FUNCTION__, core->gpr[s][src1], ~core->gpr[s][src2], core->gpr[s][dst]);
	}
	core->pc += 4;
	return 0;
}
static int exec_avg2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_avg4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_b(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int scst21 = BITS(7, 27);
		/* sign extend */
		scst21 = (scst21 & 0x100000) ? (0xFFE00000 | scst21) : scst21;
		
		DBG("In %s, delay_slot=%d, \n", __FUNCTION__, core->delay_slot);
		if(core->delay_slot != 0){
			core->pfc_bak1 = core->pce1 + (scst21 << 2);
			core->delay_slot_bak1 = 5 + 1 - core->delay_slot;
			DBG("In %s, delay_slot=%d, pfc_bak1=0x%x, delay_slot_bak1 = %d\n", __FUNCTION__, core->delay_slot, core->pfc_bak1, core->delay_slot_bak1);
			//sleep(10);
		}
		else{
			core->pfc = core->pce1 + (scst21 << 2);
			core->delay_slot = 5 + 1;
			DBG("In %s, pfc=0x%x\n", __FUNCTION__, core->pfc);
		}
	}
	core->pc += 4;
	//NOT_IMP;
	return 0;
}

static int exec_b_reg(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src2 = BITS(18, 22); 
		int x = BITS(12, 12);
		generic_address_t target_addr;
		if(x)
			target_addr = core->gpr[GPR_A][src2];
		else
			target_addr = core->gpr[GPR_B][src2];
		if(core->delay_slot != 0){
			core->pfc_bak1 = target_addr;
			core->delay_slot_bak1 = 5 + 1 - core->delay_slot;
			DBG("In %s, delay_slot=%d, pfc_bak1=0x%x, delay_slot_bak1 = %d\n", __FUNCTION__, core->delay_slot, core->pfc_bak1, core->delay_slot_bak1);
			//sleep(10);
		}
		else{
			core->pfc = target_addr;
			core->delay_slot = 5 + 1;
			DBG("In %s, pfc=0x%x\n", __FUNCTION__, core->pfc);
		}

	}
	core->pc += 4;

	return 0;
}

static int exec_b_irp(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_b_nrp(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_bdec(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_bitc4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_bitr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_bnop(c6k_core_t* core, uint32_t insn){
	int src1 = BITS(13, 15);
	if(calc_cond(core,insn)){
		//DST(insn)
		int src2 = BITS(16, 27);
		src2 = SIGN_EXTEND(src2, 12);
		if(core->header) /* compact format */
			core->pfc = core->pce1 + (src2 << 1);
		else
			core->pfc = core->pce1 + (src2 << 2);
		if(core->delay_slot == 0){
			core->delay_slot = 5 + 1;
			/* skip the specific nop */
			core->delay_slot -= src1;
			DBG("In %s, pce1 =0x%x, (src2 << 1) = 0x%x, (src2 << 2)=0x%x\n", __FUNCTION__, core->pce1, src2 << 1, src2 << 2);
			DBG("In %s, pfc = 0x%x, pc = 0x%x, src2=%d, src1=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc,src2, src1, core->delay_slot);

		}
		else{
			DBG("In %s, delay_slot=%d\n", __FUNCTION__, core->delay_slot);
			NOT_IMP;
		}
	}
	else{
		DBG("In %s, bnop not taken, pfc = 0x%x, pc = 0x%x, src1=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, src1, core->delay_slot);
		if(core->delay_slot >= src1){
			core->delay_slot -= (src1);
		}
		else
			core->delay_slot = 0;
	}
	core->pc += 4;
	return 0;
}

static int exec_bnop_reg(c6k_core_t* core, uint32_t insn){
	int src1 = BITS(13, 15);
	if(calc_cond(core,insn)){
		//DST(insn)
		int src2 = BITS(18, 22);
		//core->pc = core->gpr[GPR_B][src2];
		if(core->delay_slot == 0){
			core->pfc = core->gpr[GPR_B][src2];
			core->delay_slot = 5 + 1;
		}
		else{
			NOT_IMP;
		}
		/* skip the specific nop */
		core->delay_slot -= src1;

	}
	else{
		if(core->delay_slot)
			core->delay_slot -= (src1);
	}
	core->pc += 4;
	return 0;
}

static int exec_bpos(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_callp(c6k_core_t* core, uint32_t insn){
	//if(calc_cond(core,insn)){
		//DST(insn)
		int scst21 = BITS(7, 27);
		int s = BITS(1, 1);
		scst21 = SIGN_EXTEND(scst21, 21);
		core->pfc = core->pce1 + (scst21 << 2);
		if(insn & 0x1){
			generic_address_t addr = core->pc + 4;
			uint32_t word;
			int i = 0; /* index of one fetch packet */
			uint32_t layout = (core->header >> 21) & 0x7f;
			uint32_t pbits = core->header & 0x3FFF;

			i = ((addr  & 0xFFFFFFFC)- core->pce1) / 4;
			int parallel = insn & 0x1;

			while(parallel){
				bus_read(32, addr, &word);
				if((word >> 28) == 0xe){
					addr += 4;
					continue;
				}
				/* update header */
				if((addr & 0x1f) == 0){
					uint32_t header;
					bus_read(32, addr + 0x1c, &header);
					if((header >> 28) == 0xe){
						layout = (header >> 21) & 0x7f;
						pbits = header & 0x3FFF;
					}
					i = 0;
				}
				DBG("In %s, addr=0x%x, i = %d, layout=0x%x, pbits=0x%x, header=0x%x\n", __FUNCTION__, addr, i, layout, pbits, core->header);
				if((layout >> i) & 0x1){
					parallel = (pbits >> (i * 2)) & 0x1;
					addr += 2;
					if(parallel == 0){
						break;
					}
					parallel = (pbits >> (i * 2)) & 0x2;
					addr += 2;
					if(parallel == 0){
						break;
					}
				}
				else{
					parallel = word & 0x1;
					addr += 4;
					if(parallel == 0){
						break;
					}
				}
				i++;
			}
			core->gpr[s][3] = addr;
		}
		else{
			core->gpr[s][3] = core->pc + 4;
		}
		DBG("In %s, pfc=0x%x, return addr=0x%x\n", __FUNCTION__, core->pfc, core->gpr[s][3]);
		/* insert 5 nop */
		if(core->delay_slot >= 5)
			core->delay_slot -= 5;
		else
			core->delay_slot = 1;
	//}
	core->pc += 4;
	return 0;
}

static int exec_clr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int csta = BITS(13, 17);
		int cstb = BITS(8, 12);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		if(cstb >= csta){
			//int v = (0xFFFFFFFF << csta) & (0xFFFFFFFF >> (31 - cstb));
			int v = ((0xFFFFFFFF << (cstb + 1)) | (0xFFFFFFFF >> (32 - csta)));
			core->gpr[s][dst] = core->gpr[s][src2] & v;
			DBG("In %s, csta=%d, cstb=%d, v=0x%x, src2=%d, dst=%d\n", __FUNCTION__, csta, cstb, v, src2, dst);
		}
		else{
			NOT_IMP;
		}

	}
	//sleep(10);
	//NOT_IMP;
	core->pc += 4;
	return 0;
}

static int exec_cmpeq(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int s = BITS(1, 1);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		switch(op){
			case 0x52:
				{
					src2 = core->gpr[GPR_A][src2];
					src1 = (src1 & 0x10) ? (src1 | 0xFFFFFFE0) :src1;
					src2 = (src2 & 0x10) ? (src2 | 0xFFFFFFE0) :src2;
					core->gpr[s][dst] = (src1 == src2) ? 1 : 0;
					DBG("In %s, src1=0x%x, src2=0x%x, result=0x%x\n", __FUNCTION__, src1, src2, core->gpr[s][dst]);
				}
				break;
			case 0x53:{
					if(x){
						int i = (!s) & 0x1;
						core->gpr[s][dst] = (core->gpr[s][src1] == core->gpr[i][src2]);
					}
					else
						core->gpr[s][dst] = (core->gpr[s][src1] == core->gpr[s][src2]);
					DBG("In %s, src1=0x%x, src2=0x%x, result=0x%x\n", __FUNCTION__, src1, src2, core->gpr[s][dst]);
				}
				//sleep(10);
				break;
			default:
			
				NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}

static int exec_cmpeq2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_cmpeq4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_cmpgt(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x46){
			src1 = SIGN_EXTEND(src1, 5);	
			if(x)
				core->gpr[s][dst] = src1 > core->gpr[(!s) & 0x1][src2] ? 1: 0;
			else
				core->gpr[s][dst] = src1 > core->gpr[s][src2] ? 1: 0;
		}
		else if(op == 0x47){
			if(x)
				core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[(!s) & 0x1][src2])? 1: 0;
			else
				core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[s][src2])? 1: 0;

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_cmpgt2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmpgtu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x4f){
			if(x)
				core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[(!s) & 0x1][src2])? 1: 0;
			else
				core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[s][src2])? 1: 0;
		}
		else if(op == 0x4e){
			src1 &= 0xf;
			if(x)
				core->gpr[s][dst] = (src1 > core->gpr[(!s) & 0x1][src2])? 1: 0;
			else
				core->gpr[s][dst] = (src1 > core->gpr[s][src2])? 1: 0;

		}

		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_cmpgtu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmplt(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmplt2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmpltu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x5f){
			if(x)
				core->gpr[s][dst] = (core->gpr[s][src1] < core->gpr[(!s) & 0x1][src2])? 1: 0;
			else
				core->gpr[s][dst] = (core->gpr[s][src1] < core->gpr[s][src2])? 1: 0;
		}
		else if(op == 0x5e){
			int ucst4 = src1 & 0xf;
			if(x)
				core->gpr[s][dst] = (ucst4 < core->gpr[(!s) & 0x1][src2])? 1: 0;
			else
				core->gpr[s][dst] = (ucst4 < core->gpr[s][src2])? 1: 0;

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_cmpltu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmpy(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmpyr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmpyr1(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_cmtl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ddotp4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ddotph2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ddotph2r(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ddotpl2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ddotpl2r(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_deal(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dint(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	core->pc += 4;
	return 0;
}

static int exec_dmv(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotp2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpn2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpnrsu2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpnrus2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotprsu2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotprus2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpsu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpus4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dotpu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dpack2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_dpackx2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ext(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int csta = BITS(13, 17);
		int cstb = BITS(8, 12);

		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		core->gpr[s][dst] = (core->gpr[s][src2] << csta) >> cstb;
		if((1 << (31 - cstb)) & core->gpr[s][dst]){
			core->gpr[s][dst] |= (0xFFFFFFFF << (31 - cstb));
		}
		DBG("In %s, csta=%d, cstb=%d, (1 << (31 - cstb))=0x%x\n", __FUNCTION__, csta, cstb, (1 << (31 - cstb)));
	}
	core->pc += 4;
	//sleep(3);
	return 0;
}
static int exec_extu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int csta = BITS(13, 17);
		int cstb = BITS(8, 12);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		core->gpr[s][dst] = (core->gpr[s][src2] << csta) >> cstb;
		DBG("In %s, csta=%d, cstb=%d, \n", __FUNCTION__, csta, cstb );
	}
	core->pc += 4;
	//sleep(10);
	return 0;
}
static int exec_gmpy(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_gmpy4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_idle(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ldbu(c6k_core_t* core, uint32_t insn){
	DBG("In %s, pc=0x%x\n", __FUNCTION__, core->pc);
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(4, 6);
		int y = BITS(7, 7);
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 0);
		bus_read(8, addr, &core->gpr[s][dst]);
		if(op == 1){
			/* ldbu */
			core->gpr[s][dst] = core->gpr[s][dst] & 0xFF;
		}
		else if(op == 2){
			/* ldb with sign extend */
			if(core->gpr[s][dst] & 0x80)
				core->gpr[s][dst] = 0xFFFFFF00 | (core->gpr[s][dst] & 0xFF);
		}	
		else{
			NOT_IMP;
		}
		DBG("In %s, addr = 0x%x, data=0x%x\n", __FUNCTION__, addr, core->gpr[s][dst]);
	}
	core->pc += 4;
	return 0;
}
static int exec_ldbu_1(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_lddw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int dst = BITS(23, 27);
		int base = BITS(18, 22);
		int offset = BITS(13, 17);
		int mode = BITS(9, 12);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 3);
		bus_read(32, addr, &core->gpr[s][dst]);
		bus_read(32, addr + 4, &core->gpr[s][dst + 1]);
		//addr = calc_addr(core, base, offset , mode, y, 3);
		DBG("In %s, addr = 0x%x, data=0x%x\n", __FUNCTION__, addr, core->gpr[s][dst]);

	}
	core->pc += 4;
	return 0;
}
static int exec_ldhu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(4, 6);
		if(op == 0x0){
			int dst = BITS(23, 27);
			int base = BITS(18, 22);
			int offset = BITS(13, 17);
			int mode = BITS(9, 12);
			int y = BITS(7, 7);
			int s = BITS(1, 1);
			generic_address_t addr = calc_addr(core, base, offset , mode, y, 1);
			bus_read(16, addr, &core->gpr[s][dst]);
			DBG("In %s, addr = 0x%x, data=0x%x\n", __FUNCTION__, addr, core->gpr[s][dst]);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_ldhu_1(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ldndw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int dst = BITS(24, 27);
		int sc = BITS(23, 23);
		int base = BITS(18, 22);
		int offset = BITS(13, 17);
		int mode = BITS(9, 12);
		int y = BITS(7, 7);
		int s = BITS(1, 1);

		dst = dst << 1;
		DBG("In %s, insn = 0x%x, base=0x%x, dst=%d\n", __FUNCTION__, insn, base, dst);
		/* FIXME, sc bit need to be considered */
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 3);
		bus_read(32, addr, &core->gpr[s][dst]);
		bus_read(32, addr + 4, &core->gpr[s][dst + 1]);
		if(sc == 0){
			NOT_IMP;
		}
		/* FIXME, increase the base */
		//addr = calc_addr(core, base, offset , mode, y, 3);
	}
	core->pc += 4;
	return 0;
}
static int exec_ldnw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int dst = BITS(23, 27);
		int base = BITS(18, 22);
		int offset = BITS(13, 17);
		int mode = BITS(9, 12);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 2);
		bus_read(32, addr, &core->gpr[s][dst]);
		DBG("In %s, addr = 0x%x, data=0x%x\n", __FUNCTION__, addr, core->gpr[s][dst]);

	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_ldw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int dst = BITS(23, 27);
		int base = BITS(18, 22);
		int offset = BITS(13, 17);
		int mode = BITS(9, 12);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 2);
		bus_read(32, addr, &core->gpr[s][dst]);
		DBG("In %s, addr = 0x%x, data=0x%x, dst=%d, base%d\n", __FUNCTION__, addr, core->gpr[s][dst], dst, base);
	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_ldw_15(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ll(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_lmbd(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_max2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_maxu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_min2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_minu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhi(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhir(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhlu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhslu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhsu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhuls(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyhus(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyih(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyihr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyil(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyilr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpylh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpylhu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyli(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpylir(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpylshu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyluhs(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpysu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(7, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x1b){
			if(x)
				core->gpr[s][dst] = (core->gpr[s][src1] & 0xFFFF) * (core->gpr[(!s) & 0x1][src2] & 0xFFFF);
			else
				core->gpr[s][dst] = (core->gpr[s][src1] & 0xFFFF) * (core->gpr[s][src2] & 0xFFFF);
			DBG("In %s, src1=%d, src2=%d\n", __FUNCTION__, src1, src2);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_mpysu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyus(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpyus4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_mpy2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy2ir(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy32(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy32_64(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy32su(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy32u(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mpy32us(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mv(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mvc(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int x = BITS(12, 12);
		int crlo = BITS(18, 22);
		int dst = BITS(23, 27);
		int op = BITS(1, 11);
		if(op == 0x1d1){
			/* from register to control */
			int x = BITS(12, 12);
			int src = BITS(18, 22);
			int crlo = BITS(23, 27);
			if(crlo == 0xd){
				core->ilc = core->gpr[(!x) & 0x1][src];
				DBG("In %s, src=%d, ilc=%d\n", __FUNCTION__, src, core->ilc);
			}
			else{
				NOT_IMP;
			}
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_mvd(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mvk_s(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int dst = BITS(23, 27);
		int cst16 = BITS(7, 22);
		int s = BITS(1, 1);
		uint32_t result;
		if(cst16 & 0x8000)
			result = cst16 | 0xFFFF0000;
		else
			result = cst16;
		/* write the result to WB buffer */
		#if 0
		int pos = core->wb_result_pos;
		core->wb_index[pos] = dst + s * 32;
		core->wb_result[pos] = result;
		core->wb_result_pos++;
		DBG("In %s, record wb index=%d, result=0x%x, pos=0x%x\n", __FUNCTION__, core->wb_index[pos], result, pos);
		#endif
		write_buffer(core, dst + s * 32, result);
	}
	else{
		//NOT_IMP;
	}
	core->pc += 4;
	return 0;
}
static int exec_mvk_l(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int scst5 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		scst5 = (scst5 & 0x10) ? (scst5 | 0xFFFFFFE0) : scst5;
		int result = scst5;
		/* write the result to WB buffer */
		#if 0
		int pos = core->wb_result_pos;
		core->wb_index[pos] = dst + s * 32;
		core->wb_result[pos] = result;
		core->wb_result_pos++;
		DBG("In %s, record wb index=%d, result=0x%x\n", __FUNCTION__, core->wb_index[pos], result);
		#endif
		write_buffer(core, dst + s * 32, result);
		//core->gpr[s][dst] = scst5;
	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_mvk_d(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int scst5 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		scst5 = (scst5 & 0x10) ? (scst5 | 0xFFFFFFE0) : scst5;
		core->gpr[s][dst] = scst5;

	}
	core->pc += 4;
	return 0;
}
static int exec_mvkh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int dst = BITS(23, 27);
		int cst16 = BITS(7, 22);
		int s = BITS(1, 1);
		core->gpr[s][dst] = (cst16 << 16) | (core->gpr[s][dst] & 0xFFFF);
		DBG("In %s, dst=%d\n", __FUNCTION__, dst);
	}
	core->pc += 4;
	return 0;
}
static int exec_mvklh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_mvkl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_neg(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_nop(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src = BITS(13, 16);
		if(core->delay_slot > src)
			core->delay_slot -= src;
		else
			core->delay_slot = 0;
	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_norm(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_not(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_or_d(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 9);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);

		if(op == 3){
			src1 = SIGN_EXTEND(src1, 5); 
			if(x)
				core->gpr[s][dst] = src1 | core->gpr[(!s) & 0x1][src2];
			else
				core->gpr[s][dst] = src1 | core->gpr[s][src2];
			
			DBG("In %s, s=%d, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, s, dst, src1, core->gpr[GPR_A][src2]);

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_or_l(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x7f){
			// if(x)
			//core->gpr[s][dst] = core->gpr[GPR_A][src1] & core->gpr[GPR_A][src2];
			if(x)
				core->gpr[s][dst] = core->gpr[s][src1] | core->gpr[(!s) & 0x1][src2];
			else
				core->gpr[s][dst] = core->gpr[s][src1] | core->gpr[s][src2];
			
			DBG("In %s, s=%d, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, s, dst, src1, core->gpr[GPR_A][src2]);
		}
		else if(op == 0x7e){
			src1 = (src1 & 0x10) ? (src1 | 0xFFFFFFe0) : src1; 
			if(x)
				core->gpr[s][dst] = src1 | core->gpr[(!s) & 0x1][src2];
			else
				core->gpr[s][dst] = src1 | core->gpr[s][src2];
			
			DBG("In %s, s=%d, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, s, dst, src1, core->gpr[GPR_A][src2]);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}

static int exec_or_s(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		if(op == 0x1a){
			src1 = SIGN_EXTEND(src1, 5); 
			if(x)
				core->gpr[s][dst] = src1 | core->gpr[(!s) & 0x1][src2];
			else
				core->gpr[s][dst] = src1 | core->gpr[s][src2];
			
			DBG("In %s, s=%d, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, s, dst, src1, core->gpr[GPR_A][src2]);

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}

static int exec_pack2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(x){
			core->gpr[s][dst] = (core->gpr[(~s) & 0x1][src2] & 0xFFFF) | ((core->gpr[s][src1] & 0xFFFF) << 16);
		}
		else{
			core->gpr[s][dst] = (core->gpr[s][src2] & 0xFFFF) | ((core->gpr[s][src1] & 0xFFFF) << 16);
		}

	}
	core->pc += 4;
	return 0;
}
static int exec_packh2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_packh4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_packhl2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_packlh2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_packl4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		int upper_half = ((core->gpr[s][src1] << 8) & 0xFF000000) | ((core->gpr[s][src1] << 16) & 0x00FF0000);
		int lower_half;
		if(x)
			lower_half = ((core->gpr[(~s) & 0x1][src2] >> 8) & 0xFF00) | (core->gpr[(~s) & 0x1][src2] & 0xFF);
		else
			lower_half = ((core->gpr[s][src2] >> 8) & 0xFF00) | (core->gpr[s][src2] & 0xFF);
		core->gpr[s][dst] = upper_half | lower_half;

	}
	core->pc += 4;
	return 0;
}
static int exec_rint(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_rotl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_rpack2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sadd(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sadd2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_saddsub(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_saddsub2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_saddsu2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_saddus2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_saddu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sat(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_set(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int csta = BITS(13, 17);
		int cstb = BITS(8, 12);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		if(cstb >= csta){
			int v = ~((0xFFFFFFFF << (cstb + 1)) | (0xFFFFFFFF >> (32 - csta)));
			core->gpr[s][dst] = core->gpr[s][src2] | v;
			DBG("In %s, csta=%d, cstb=%d, v=0x%x, src2=%d, dst=%d, result=0x%x\n", __FUNCTION__, csta, cstb, v, src2, dst, core->gpr[s][dst]);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	//sleep(10);
	return 0;
}
static int exec_shfl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_shfl3(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_shl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		if(op == 0x32){
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] << src1;
			else
				core->gpr[s][dst] = core->gpr[s][src2] << src1;
			DBG("In %s, src1=0x%x, src2=0x%x\n", __FUNCTION__, src1, src2);
		}
		else{
			NOT_IMP;
		}

	}
	core->pc += 4;
	return 0;
}
static int exec_shlmb(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_shr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x36){
			if(x)
				core->gpr[s][dst] = (core->gpr[(!s) & 0x1][src2] & 0xFFFFFF) >> src1;
			else
				core->gpr[s][dst] = (core->gpr[s][src2] & 0xFFFFFF) >> src1;
			DBG("In %s, before sign-extend, result=0x%x\n", __FUNCTION__, core->gpr[s][dst]);
			if(core->gpr[s][dst] & 0x80000000)
				core->gpr[s][dst] |= (0xFFFFFFFF << (32 - src1));
				
			DBG("In %s, after sign-extend, result=0x%x\n", __FUNCTION__, core->gpr[s][dst]);
			//sleep(10);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_shr2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_shrmb(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_shru(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(6, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int x = BITS(12, 12);
		int s = BITS(1, 1);
		if(op == 0x26){
			/* ucst5 */
			src1 = src1 & 0x1F;
			if(x)
				core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] >> src1;
			else
				core->gpr[s][dst] = core->gpr[s][src2] >> src1;
			DBG("In %s, src1=%d, dst=%d, result=0x%x\n", __FUNCTION__, src1, dst, core->gpr[s][dst]);
		}
		else{	
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_shru2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpy(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpyh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpyhl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpylh(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpy2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_smpy32(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_spack2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_spacku4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_spkernel(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		core->sploop_end = core->pc;
		int fstg_fcyc = BITS(22, 27);
		core->ilc --;
		DBG("In %s, ilc=0x%x\n", __FUNCTION__, core->ilc);
	}
	//sleep(5);
	core->pc += 4;
	return 0;
}
static int exec_spkernelr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sploop(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int ii = BITS(23, 27);
		int p = BITS(0, 0);
		if(p)
			core->sploop_begin = core->pc + 8;
		else
			core->sploop_begin = core->pc + 4;
		//core->sploop_end = core->sploop_begin + SPLOOP_BUFFER_SIZE * 4;
		DBG("begin loop at 0x%x, ilc=%d\n", core->pc, core->ilc);
	}
	core->pc += 4;
	//NOT_IMP;
	return 0;
}
static int exec_sploopd(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sploopw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int ii = BITS(23, 27);
		int p = BITS(0, 0);
		if(p)
			core->sploop_begin = core->pc + 8;
		else
			core->sploop_begin = core->pc + 4;
		core->sploopw_cond = insn;
		core->sploopw_flag = 1;
		//core->sploop_end = core->sploop_begin + SPLOOP_BUFFER_SIZE * 4;
		DBG("begin loopw at 0x%x, cond=%d\n", core->pc, core->sploopw_cond);

	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_spmask(c6k_core_t* core, uint32_t insn){
		//DST(insn)
	core->spmask = 1;
	uint32_t addr = core->pc;
	uint32_t word;
	int pbits = 0;
	int layout = 0;
	int i = 0; /* index of one fetch packet */
	layout = (core->header >> 21) & 0x7f;
	pbits = core->header & 0x3FFF;

	int parallel = insn & 0x1;
	/* next instruction */
	addr += 4;
	i = ((addr  & 0xFFFFFFFC)- core->pce1) / 4;
	core->spmask_begin = addr;
	while(parallel){
		bus_read(32, addr, &word);
		if((word >> 28) == 0xe){
			addr += 4;
			continue;
		}
		/* update header */
		if((addr & 0x1f) == 0){
			uint32_t header;
			bus_read(32, addr + 0x1c, &header);
			if((header >> 28) == 0xe){
				layout = (header >> 21) & 0x7f;
				pbits = header & 0x3FFF;
			}
			i = 0;
		}
		if((layout >> i) & 0x1){
			parallel = (pbits >> (i * 2)) & 0x1;
			addr += 2;
			if(parallel == 0){
				break;
			}
			addr += 2;
			parallel = (pbits >> (i * 2)) & 0x2;
			if(parallel == 0){
				break;
			}
		}
		else{
			parallel = word & 0x1;
			addr += 4;
			if(parallel == 0){
				break;
			}
		}
		i++;
	}
	core->spmask_end = addr;
	DBG("In %s, spmask_begin=0x%x, spmask_end=0x%x\n", __FUNCTION__, core->spmask_begin, core->spmask_end);
	//sleep(10);
	core->pc += 4;
	return 0;
}
static int exec_spmaskr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sshl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sshvl(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sshvr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ssub(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_ssub2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_stb(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(4, 6);
		int y = BITS(7, 7);
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(23, 27);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset , mode, y, 0);
		bus_write(8, addr, core->gpr[s][src] & 0xFF);
		DBG("In %s, addr = 0x%x, data=0x%x\n", __FUNCTION__, addr, core->gpr[s][src]);
		if(addr >= 0x817ded && addr < (0x817ded + 0x10)){
			char c = core->gpr[s][src] & 0xFF;
			skyeye_uart_write(0, &c, 1, NULL);
		}

	}
	//NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_stb_15(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_stdw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(24, 27) << 1;
		int sc = BITS(23, 23);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset, mode, y, 3);
		/* sc need to be considered */
		bus_write(32, addr, core->gpr[s][src]);
		bus_write(32, addr + 4, core->gpr[s][src + 1]);
		/* FIXME, just for inc/dec base twice for dword */
		//addr = calc_addr(core, base, offset, mode, y, 3);
		DBG("In %s, addr=0x%x, src=%d, base=%d\n", __FUNCTION__, addr, src, base);
		//sleep(10);
	}
	core->pc += 4;
	return 0;
}
static int exec_sth(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(23, 27);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset, mode, y, 1);
		bus_write(16, addr, core->gpr[s][src] & 0xFFFF);

	}
	core->pc += 4;
	return 0;
}
static int exec_sth_15(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		}
	NOT_IMP;
	return 0;
}
static int exec_stndw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(24, 27) << 1;
		int sc = BITS(23, 23);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset, mode, y, 3);
		/* sc need to be considered */
		bus_write(32, addr, core->gpr[s][src]);
		bus_write(32, addr + 4, core->gpr[s][src + 1]);
		/* FIXME, just for inc/dec base twice for dword */
		//addr = calc_addr(core, base, offset, mode, y, 3);
		DBG("In %s, addr=0x%x, a[5]=0x%x\n", __FUNCTION__, addr, core->gpr[s][5]);
		if(sc == 0){ /* should not scale */
			NOT_IMP;
		}

		//DST(insn)

	}
	core->pc += 4;
	return 0;
}
static int exec_stnw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(23, 27);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset, mode, y, 2);
		DBG("In %s, addr=0x%x\n", __FUNCTION__, addr);
		bus_write(32, addr, core->gpr[s][src]);
		//DST(insn)

	}
	core->pc += 4;
	return 0;
}
static int exec_stw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		int mode = BITS(9, 12);
		int offset = BITS(13, 17);
		int base = BITS(18, 22);
		int src = BITS(23, 27);
		int y = BITS(7, 7);
		int s = BITS(1, 1);
		generic_address_t addr = calc_addr(core, base, offset, mode, y, 2);
		DBG("In %s, addr=0x%x, src=%d, value=0x%x\n", __FUNCTION__, addr, src, core->gpr[s][src]);
		bus_write(32, addr, core->gpr[s][src]);
		//DST(insn)
	}
	//else
	//	NOT_IMP;
	core->pc += 4;
	return 0;
}
static int exec_stw_15(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sub(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(5, 11);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		int x = BITS(12, 12);
		if(op == 0x27){
			src2 = SIGN_EXTEND(src2, 5);
			uint64_t result;
			if(x){
				result = core->gpr[s][src1] - core->gpr[(!s) & 0x1][src2];
				/* dst is slong type */
				core->gpr[s][dst] = result & 0xFFFFFFFF;
				core->gpr[s][dst + 1] = (result >> 32) & 0xFF;
			}
			else{
				result = core->gpr[s][src1] - core->gpr[s][src2];
				/* dst is slong type */
				core->gpr[s][dst] = result & 0xFFFFFFFF;
				core->gpr[s][dst + 1] = (result >> 32) & 0xFF;
			}
			DBG("In %s, ZERO dst=%d, src1=%d, src2=%d, result=0x%x\n", __FUNCTION__, dst, src1, src2, result);
		}
		else if(op == 0x7){
			int result;
			if(x){
				result = core->gpr[s][src1] - core->gpr[(!s) & 0x1][src2];
				core->gpr[s][dst] = result;
			}
			else{
				result = core->gpr[s][src1] - core->gpr[s][src2];
				/* dst is slong type */
				core->gpr[s][dst] = result;
			}
			DBG("In %s, dst=%d, src1=%d, src2=%d, result=0x%x\n", __FUNCTION__, dst, src1, src2, result);
		}
		else if(op == 0x6){
			src1 = SIGN_EXTEND(src1, 5);
			int result;
			if(x){
				result = src1 - core->gpr[(!s) & 0x1][src2];
				core->gpr[s][dst] = result;
			}
			else{
				result = src1 - core->gpr[s][src2];
				/* dst is slong type */
				core->gpr[s][dst] = result;
			}
			DBG("In %s, dst=%d, src1=%d, src2=%d, result=0x%x\n", __FUNCTION__, dst, src1, src2, result);

		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}

static int exec_sub_d(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
		int op = BITS(7, 12);
		int src1 = BITS(13, 17);
		int src2 = BITS(18, 22);
		int dst = BITS(23, 27);
		int s = BITS(1, 1);
		if(op == 0x11){
			core->gpr[s][dst] = core->gpr[s][src2] - core->gpr[s][src1];
			DBG("In %s, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, dst, src1, src2);
		}
		else if(op == 0x13){
			core->gpr[s][dst] = core->gpr[s][src2] -src1;
			DBG("In %s, dst=%d, src1=%d, src2=%d\n", __FUNCTION__, dst, src1, src2);
		}
		else{
			NOT_IMP;
		}
	}
	core->pc += 4;
	return 0;
}
static int exec_sub_d_x(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

static int exec_subab(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_subabs4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_subah(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_subaw(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_subc(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_subu(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sub2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_sub4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_swap2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_swap4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_swe(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_swenr(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_unpkhu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_unpklu4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_xor(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_xormpy(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_xpnd2(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_xpnd4(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}
static int exec_zero(c6k_core_t* core, uint32_t insn){
	if(calc_cond(core,insn)){
		//DST(insn)
	}
	NOT_IMP;
	return 0;
}

const ISEITEM insn32_decode_table[] = {
{"abs", 3, 6, 2, 4, 0x6, 5, 11, 0x1a, 13, 17, 0x0},
{"abs", 3, 6, 2, 4, 0x6, 5, 11, 0x38, 13, 17, 0x0},
{"abs2", 2, 6, 2, 11, 0xd6, 13, 17, 0x4},

{"add_l", 2, 6, 2, 4, 0x6, 5, 11, 0x3},
{"add_l", 2, 6, 2, 4, 0x6, 5, 11, 0x23},
{"add_l", 2, 6, 2, 4, 0x6, 5, 11, 0x21},
{"add_l", 2, 6, 2, 4, 0x6, 5, 11, 0x2},
{"add_l", 2, 6, 2, 4, 0x6, 5, 11, 0x20},

{"add_s", 2, 6, 2, 5, 0x8, 6, 11, 0x7},
{"add_s", 2, 6, 2, 5, 0x8, 6, 11, 0x6},

{"add_d", 2, 6, 2, 6, 0x10, 7, 12, 0x10},
{"add_d", 2, 6, 2, 6, 0x10, 7, 12, 0x12},
{"add_dx", 1, 6, 2, 11, 0x2ac},
{"add_dxc", 1, 6, 2, 11, 0x2bc},

{"addab", 0, 6, 2, 4, 0x6},
{"addad", 2, 6, 2, 6, 0x10, 7 ,12, 0x3c},
{"addad", 2, 6, 2, 6, 0x10, 7 ,12, 0x3d},
{"addah", 0, 6, 2, 4, 0x6},
{"addaw", 0, 6, 2, 4, 0x6},
{"addk", 1, 6, 2, 6, 0x14},
{"addkpc", 1, 6, 2, 12, 0x58},
{"addsub", 0, 6, 2, 4, 0x6},
{"addsub2", 0, 6, 2, 4, 0x6},
{"addu", 0, 6, 2, 4, 0x6},
{"add2", 0, 6, 2, 4, 0x6},
{"add4", 0, 6, 2, 4, 0x6},

{"and_l", 2, 6, 2, 4, 0x6, 5, 11, 0x7b},
{"and_l", 2, 6, 2, 4, 0x6, 5, 11, 0x7a},
{"and_s", 1, 6, 2, 11, 0x1f8},
{"and_s_cst", 1, 6, 2, 11, 0x1e8},
{"and_d", 3, 6, 2, 5, 0xc, 6, 9, 0x6, 10, 11, 0x2},
{"and_d", 3, 6, 2, 5, 0xc, 6, 9, 0x7, 10, 11, 0x2},
{"andn", 1, 6, 2, 11, 0x3e6},
{"andn", 1, 6, 2, 11, 0x36c},
{"andn", 1, 6, 2, 11, 0x20c},
{"avg2", 0, 6, 2, 4, 0x6},
{"avg4", 0, 6, 2, 4, 0x6},
/* avoid decode with callp instruction */
{"b", 2, 6, 2, 6, 0x4, 28 ,31, 0x0},
{"b", 2, 6, 2, 6, 0x4, 29, 31, 0x1},
{"b", 2, 6, 2, 6, 0x4, 29, 31, 0x2},
{"b", 2, 6, 2, 6, 0x4, 29 ,31, 0x3},
{"b", 2, 6, 2, 6, 0x4, 29, 31, 0x4},
{"b", 2, 6, 2, 6, 0x4, 29, 31, 0x5},
{"b", 2, 6, 2, 6, 0x4, 29, 31, 0x6},

{"b_reg", 3, 6, 1, 11, 0x1b1, 13, 17, 0x0, 23, 27, 0x0},
{"b_irp", 0, 6, 2, 4, 0x6},
{"b_nrp", 0, 6, 2, 4, 0x6},
{"bdec", 0, 6, 2, 4, 0x6},

{"bitc4", 0, 6, 2, 4, 0x6},
{"bitr", 0, 6, 2, 4, 0x6},
{"bnop", 1, 6, 2, 12, 0x48},
{"bnop_reg", 3, 6, 1, 11, 0x1b1, 16, 17, 0x0, 23, 27, 0x1},
{"bpos", 0, 6, 2, 4, 0x6},

{"callp", 2, 6, 2, 6, 0x4, 28, 31, 0x1},
{"clr", 1, 6, 2, 7, 0x32},

{"cmpeq", 2, 6, 2, 4, 0x6, 5, 11, 0x53},
{"cmpeq", 2, 6, 2, 4, 0x6, 5, 11, 0x52},
{"cmpeq", 2, 6, 2, 4, 0x6, 5, 11, 0x51},
{"cmpeq", 2, 6, 2, 4, 0x6, 5, 11, 0x50},

{"cmpeq2", 0, 6, 2, 4, 0x6},
{"cmpeq4", 0, 6, 2, 4, 0x6},
{"cmpgt", 2, 6, 2, 4, 0x6, 5, 11, 0x44},
{"cmpgt", 2, 6, 2, 4, 0x6, 5, 11, 0x45},
{"cmpgt", 2, 6, 2, 4, 0x6, 5, 11, 0x46},
{"cmpgt", 2, 6, 2, 4, 0x6, 5, 11, 0x47},

{"cmpgt2", 0, 6, 2, 4, 0x6},
{"cmpgtu", 2, 6, 2, 4, 0x6, 5, 11, 0x4c},
{"cmpgtu", 2, 6, 2, 4, 0x6, 5, 11, 0x4d},
{"cmpgtu", 2, 6, 2, 4, 0x6, 5, 11, 0x4e},
{"cmpgtu", 2, 6, 2, 4, 0x6, 5, 11, 0x4f},
{"cmpgtu4", 0, 6, 2, 4, 0x6},
{"cmplt", 0, 6, 2, 4, 0x6},
{"cmplt2", 0, 6, 2, 4, 0x6},
{"cmpltu", 2, 6, 2, 4, 0x6, 5, 11, 0x5c},
{"cmpltu", 2, 6, 2, 4, 0x6, 5, 11, 0x5d},
{"cmpltu", 2, 6, 2, 4, 0x6, 5, 11, 0x5e},
{"cmpltu", 2, 6, 2, 4, 0x6, 5, 11, 0x5f},
{"cmpltu4", 0, 6, 2, 4, 0x6},

{"cmpy", 0, 6, 2, 4, 0x6},
{"cmpyr", 0, 6, 2, 4, 0x6},
{"cmpyr1", 0, 6, 2, 4, 0x6},
{"cmtl", 0, 6, 2, 4, 0x6},

{"ddotp4", 0, 6, 2, 4, 0x6},
{"ddotph2", 0, 6, 2, 4, 0x6},
{"ddotph2r", 0, 6, 2, 4, 0x6},
{"ddotpl2", 0, 6, 2, 4, 0x6},
{"ddotpl2r", 0, 6, 2, 4, 0x6},

{"deal", 0, 6, 2, 4, 0x6},
{"dint", 1, 6, 1, 31, 0x8002000},
{"dmv", 0, 6, 2, 4, 0x6},

{"dotp2", 0, 6, 2, 4, 0x6},
{"dotpn2", 0, 6, 2, 4, 0x6},
{"dotpnrsu2", 0, 6, 2, 4, 0x6},
{"dotpnrus2", 0, 6, 2, 4, 0x6},
{"dotprsu2", 0, 6, 2, 4, 0x6},
{"dotprus2", 0, 6, 2, 4, 0x6},
{"dotpsu4", 0, 6, 2, 4, 0x6},
{"dotpus4", 0, 6, 2, 4, 0x6},
{"dotpu4", 0, 6, 2, 4, 0x6},

{"dpack2", 0, 6, 2, 4, 0x6},
{"dpackx2", 0, 6, 2, 4, 0x6},
{"ext", 1, 6, 2, 7, 0x12},
{"extu", 1, 6, 2, 7, 0x2},
{"gmpy", 0, 6, 2, 4, 0x6},
{"gmpy4", 0, 6, 2, 4, 0x6},
{"idle", 0, 6, 2, 4, 0x6},

{"ldbu", 3, 6, 2, 3, 0x1, 4, 6, 0x1, 8, 8, 0x0},
{"ldbu", 3, 6, 2, 3, 0x1, 4, 6, 0x2, 8, 8, 0x0},
{"ldbu_1", 0, 6, 2, 4, 0x6},
{"lddw", 2, 6, 2, 6, 0x19, 8, 8, 0x1},
{"ldhu", 2, 6, 2, 3, 0x1, 4, 6, 0x0},
{"ldhu_1", 0, 6, 2, 4, 0x6},
{"ldndw", 2, 6, 2, 6, 0x9, 8, 8, 1},
{"ldnw", 2, 6, 2, 6, 0xd, 8, 8, 1},
{"ldw", 2, 6, 2, 6, 0x19, 8, 8, 0},
{"ldw_15", 1, 6, 2, 6, 0x1b},

{"ll", 0, 6, 2, 4, 0x6},
{"lmbd", 0, 6, 2, 4, 0x6},
{"max2", 0, 6, 2, 4, 0x6},
{"maxu4", 0, 6, 2, 4, 0x6},
{"min2", 0, 6, 2, 4, 0x6},
{"minu4", 0, 6, 2, 4, 0x6},

{"mpy", 0, 6, 2, 4, 0x6},
{"mpyh", 0, 6, 2, 4, 0x6},
{"mpyhi", 0, 6, 2, 4, 0x6},
{"mpyhir", 0, 6, 2, 4, 0x6},
{"mpyhl", 0, 6, 2, 4, 0x6},
{"mpyhlu", 0, 6, 2, 4, 0x6},
{"mpyhslu", 0, 6, 2, 4, 0x6},
{"mpyhsu", 0, 6, 2, 4, 0x6},
{"mpyhu", 0, 6, 2, 4, 0x6},
{"mpyhuls", 0, 6, 2, 4, 0x6},
{"mpyhus", 0, 6, 2, 4, 0x6},

{"mpyih", 0, 6, 2, 4, 0x6},
{"mpyihr", 0, 6, 2, 4, 0x6},
{"mpyil", 0, 6, 2, 4, 0x6},
{"mpyilr", 0, 6, 2, 4, 0x6},
{"mpylh", 0, 6, 2, 4, 0x6},
{"mpylhu", 0, 6, 2, 4, 0x6},
{"mpyli", 0, 6, 2, 4, 0x6},
{"mpylir", 0, 6, 2, 4, 0x6},
{"mpylshu", 0, 6, 2, 4, 0x6},
{"mpyluhs", 0, 6, 2, 4, 0x6},
{"mpysu", 2, 6, 2, 6, 0x0, 7, 11, 0x1b},
{"mpysu", 2, 6, 2, 6, 0x0, 7, 11, 0x1e},
{"mpysu4", 0, 6, 2, 4, 0x6},

{"mpyu", 0, 6, 2, 4, 0x6},
{"mpyu4", 0, 6, 2, 4, 0x6},
{"mpyus", 0, 6, 2, 4, 0x6},
{"mpyus4", 0, 6, 2, 4, 0x6},
{"mpy2", 0, 6, 2, 4, 0x6},
{"mpy2ir", 0, 6, 2, 4, 0x6},
{"mpy32", 0, 6, 2, 4, 0x6},
{"mpy32_64", 0, 6, 2, 4, 0x6},
{"mpy32su", 0, 6, 2, 4, 0x6},
{"mpy32u", 0, 6, 2, 4, 0x6},
{"mpy32us", 0, 6, 2, 4, 0x6},

{"mv", 0, 6, 2, 4, 0x6},
{"mvc", 2, 6, 1, 11, 0x1f1, 13, 17, 0x0},
{"mvc", 2, 6, 1, 11, 0x1d1, 13, 17, 0x0},
{"mvd", 0, 6, 2, 4, 0x6},
{"mvk_s", 1, 6, 2, 6, 0xa},
{"mvk_l", 2, 6, 2, 11, 0xd6, 13, 17, 0x5},
{"mvk_d", 2, 6, 2, 12, 0x10, 18, 22, 0x0},
{"mvkh", 1, 6, 2, 5, 0xa},
{"mvklh", 0, 6, 2, 4, 0x6},
{"mvkl", 0, 6, 2, 4, 0x6},

{"neg", 0, 6, 2, 4, 0x6},
{"nop", 2, 6, 1, 12, 0x0, 17, 31, 0x0},
{"norm", 0, 6, 2, 4, 0x6},
{"not", 0, 6, 2, 4, 0x6},
{"or_d", 3, 6, 2, 5, 0xc, 6, 9, 0x2, 10, 11, 0x2},
{"or_d", 3, 6, 2, 5, 0xc, 6, 9, 0x3, 10, 11, 0x2},
{"or_l", 2, 6, 2, 4, 0x6, 5, 11, 0x7f},
{"or_l", 2, 6, 2, 4, 0x6, 5, 11, 0x7e},
{"or_s", 2, 6, 2, 5, 0x8, 6 ,11, 0x1a},
{"or_s", 2, 6, 2, 5, 0x8, 6 ,11, 0x1b},

{"pack2", 1, 6, 2, 11, 0x6},
{"pack2", 1, 6, 2, 11, 0x3fc},

{"packh2", 0, 6, 2, 4, 0x6},
{"packh4", 0, 6, 2, 4, 0x6},
{"packhl2", 0, 6, 2, 4, 0x6},
{"packlh2", 0, 6, 2, 4, 0x6},
{"packl4", 1, 6, 2, 11, 0x346},
{"rint", 0, 6, 2, 4, 0x6},
{"rotl", 0, 6, 2, 4, 0x6},
{"rpack2", 0, 6, 2, 4, 0x6},

{"sadd", 0, 6, 2, 4, 0x6},
{"sadd2", 0, 6, 2, 4, 0x6},
{"saddsub", 0, 6, 2, 4, 0x6},
{"saddsub2", 0, 6, 2, 4, 0x6},
{"saddsu2", 0, 6, 2, 4, 0x6},
{"saddus2", 0, 6, 2, 4, 0x6},
{"saddu4", 0, 6, 2, 4, 0x6},

{"sat", 0, 6, 2, 4, 0x6},
{"set", 1, 6, 2, 7, 0x22},
{"shfl", 0, 6, 2, 4, 0x6},
{"shfl3", 0, 6, 2, 4, 0x6},
{"shl", 2, 6, 2, 5, 0x8, 6, 11, 0x32},
{"shlmb", 0, 6, 2, 4, 0x6},
{"shr", 2, 6, 2, 5, 0x8, 6, 11, 0x36},
{"shr2", 0, 6, 2, 4, 0x6},
{"shrmb", 0, 6, 2, 4, 0x6},

{"shru", 2, 6, 2, 5, 0x8, 6, 11, 0x27},
{"shru", 2, 6, 2, 5, 0x8, 6, 11, 0x25},
{"shru", 2, 6, 2, 5, 0x8, 6, 11, 0x26},
{"shru", 2, 6, 2, 5, 0x8, 6, 11, 0x24},
{"shru2", 0, 6, 2, 4, 0x6},
{"sl", 0, 6, 2, 4, 0x6},

{"smpy", 0, 6, 2, 4, 0x6},
{"smpyh", 0, 6, 2, 4, 0x6},
{"smpyhl", 0, 6, 2, 4, 0x6},
{"smpylh", 0, 6, 2, 4, 0x6},
{"smpy2", 0, 6, 2, 4, 0x6},
{"smpy32", 0, 6, 2, 4, 0x6},
{"spack2", 0, 6, 2, 4, 0x6},
{"spacku4", 0, 6, 2, 4, 0x6},
{"spkernel", 2, 6, 2, 21, 0xd000, 28, 31, 0x0},
{"spkernelr", 0, 6, 2, 4, 0x6},
{"sploop", 1, 6, 2, 22, 0xe000},
{"sploopd", 0, 6, 2, 4, 0x6},
{"sploopw", 1, 6, 2, 22, 0xf800},
{"spmask", 2, 6, 2, 17, 0xc000, 26, 31, 0x0},
{"spmaskr", 0, 6, 2, 4, 0x6},
{"sshl", 0, 6, 2, 4, 0x6},
{"sshvl", 0, 6, 2, 4, 0x6},
{"sshvr", 0, 6, 2, 4, 0x6},
{"ssub", 0, 6, 2, 4, 0x6},
{"ssub2", 0, 6, 2, 4, 0x6},

{"stb", 2, 6, 2, 6, 0xd, 8, 8, 0x0},
{"stb_15", 0, 6, 2, 4, 0x6},
{"stdw", 2, 6, 2, 6, 0x11, 8, 8, 0x1},
{"sth", 2, 6, 2, 6, 0x15, 8, 8, 0x0},
{"sth_15", 0, 6, 2, 4, 0x6},
{"stndw", 2, 6, 2, 6, 0x1d, 8, 8, 0x1},
{"stnw", 2, 6, 2, 6, 0x15, 8, 8, 0x1},
{"stw", 2, 6, 2, 6, 0x1d, 8, 8, 0x0},
{"stw_15", 0, 6, 2, 4, 0x6},

{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x7},
{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x17},
{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x27},
{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x37},
{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x6},
{"sub", 2, 6, 2, 4, 0x6, 5, 11, 0x24},
{"sub_d_x", 1, 6, 2, 11, 0x2cc}, /* sub D, cross path */

{"sub_d", 2, 6, 2, 6, 0x10, 7, 12, 0x11}, /* sub D, not cross path */
{"sub_d", 2, 6, 2, 6, 0x10, 7, 12, 0x13}, /* sub D, not cross path */

{"subab", 0, 6, 2, 4, 0x6},
{"subabs4", 0, 6, 2, 4, 0x6},
{"subah", 0, 6, 2, 4, 0x6},
{"subaw", 0, 6, 2, 4, 0x6},
{"subc", 0, 6, 2, 4, 0x6},
{"subu", 0, 6, 2, 4, 0x6},
{"sub2", 0, 6, 2, 4, 0x6},
{"sub4", 0, 6, 2, 4, 0x6},

{"swap2", 0, 6, 2, 4, 0x6},
{"swap4", 0, 6, 2, 4, 0x6},
{"swe", 0, 6, 2, 4, 0x6},
{"swenr", 0, 6, 2, 4, 0x6},
{"unpkhu4", 0, 6, 2, 4, 0x6},
{"unpklu4", 0, 6, 2, 4, 0x6},
{"xor", 0, 6, 2, 4, 0x6},
{"xormpy", 0, 6, 2, 4, 0x6},
{"xpnd2", 0, 6, 2, 4, 0x6},
{"xpnd4", 0, 6, 2, 4, 0x6},
{"zero", 0, 6, 2, 4, 0x6},
};

insn_action_t insn_action[] = {
	exec_abs,
	exec_abs,
	exec_abs2,

	exec_add_l,
	exec_add_l,
	exec_add_l,
	exec_add_l,
	exec_add_l,

	exec_add_s,
	exec_add_s,

	exec_add_d,
	exec_add_d,
	exec_add_d,
	exec_add_d,

	exec_addab,
	exec_addad,
	exec_addad,
	exec_addah,
	exec_addaw,
	exec_addk,
	exec_addkpc,
	exec_addsub,
	exec_addsub2,
	exec_addu,
	exec_add2,
	exec_add4,

	exec_and_l,
	exec_and_l,
	exec_and_s,
	exec_and_s_cst,
	exec_and_d,
	exec_and_d,

	exec_andn,
	exec_andn,
	exec_andn,

	exec_avg2,
	exec_avg4,

	exec_b,
	exec_b,
	exec_b,
	exec_b,
	exec_b,
	exec_b,
	exec_b,

	exec_b_reg,
	exec_b_irp,
	exec_b_nrp,
	exec_bdec,

	exec_bitc4,
	exec_bitr,
	exec_bnop,
	exec_bnop_reg,
	exec_bpos,

	exec_callp,
	exec_clr,

	exec_cmpeq,
	exec_cmpeq,
	exec_cmpeq,
	exec_cmpeq,

	exec_cmpeq2,
	exec_cmpeq4,
	exec_cmpgt,
	exec_cmpgt,
	exec_cmpgt,
	exec_cmpgt,

	exec_cmpgt2,

	exec_cmpgtu,
	exec_cmpgtu,
	exec_cmpgtu,
	exec_cmpgtu,

	exec_cmpgtu4,
	exec_cmplt,
	exec_cmplt2,
	exec_cmpltu,
	exec_cmpltu,
	exec_cmpltu,
	exec_cmpltu,

	exec_cmpltu4,

	exec_cmpy,
	exec_cmpyr,
	exec_cmpyr1,
	exec_cmtl,

	exec_ddotp4,
	exec_ddotph2,
	exec_ddotph2r,
	exec_ddotpl2,
	exec_ddotpl2r,

	exec_deal,
	exec_dint,
	exec_dmv,

	exec_dotp2,
	exec_dotpn2,
	exec_dotpnrsu2,
	exec_dotpnrus2,
	exec_dotprsu2,
	exec_dotprus2,
	exec_dotpsu4,
	exec_dotpus4,
	exec_dotpu4,

	exec_dpack2,
	exec_dpackx2,
	exec_ext,
	exec_extu,
	exec_gmpy,
	exec_gmpy4,
	exec_idle,

	exec_ldbu,
	exec_ldbu,
	exec_ldbu_1,
	exec_lddw,
	exec_ldhu,
	exec_ldhu_1,
	exec_ldndw,
	exec_ldnw,
	exec_ldw,
	exec_ldw_15,

	exec_ll,
	exec_lmbd,
	exec_max2,
	exec_maxu4,
	exec_min2,
	exec_minu4,

	exec_mpy,
	exec_mpyh,
	exec_mpyhi,
	exec_mpyhir,
	exec_mpyhl,
	exec_mpyhlu,
	exec_mpyhslu,
	exec_mpyhsu,
	exec_mpyhu,
	exec_mpyhuls,
	exec_mpyhus,

	exec_mpyih,
	exec_mpyihr,
	exec_mpyil,
	exec_mpyilr,
	exec_mpylh,
	exec_mpylhu,
	exec_mpyli,
	exec_mpylir,
	exec_mpylshu,
	exec_mpyluhs,

	exec_mpysu,
	exec_mpysu,
	exec_mpysu4,

	exec_mpyu,
	exec_mpyu4,
	exec_mpyus,
	exec_mpyus4,
	exec_mpy2,
	exec_mpy2ir,
	exec_mpy32,
	exec_mpy32_64,
	exec_mpy32su,
	exec_mpy32u,
	exec_mpy32us,

	exec_mv,
	exec_mvc,
	exec_mvc,

	exec_mvd,
	exec_mvk_s,
	exec_mvk_l,
	exec_mvk_d,
	exec_mvkh,
	exec_mvklh,
	exec_mvkl,

	exec_neg,
	exec_nop,
	exec_norm,
	exec_not,

	exec_or_d,
	exec_or_d,
	exec_or_l,
	exec_or_l,
	exec_or_s,
	exec_or_s,

	exec_pack2,
	exec_pack2,
	exec_packh2,
	exec_packh4,
	exec_packhl2,
	exec_packlh2,
	exec_packl4,
	exec_rint,
	exec_rotl,
	exec_rpack2,

	exec_sadd,
	exec_sadd2,
	exec_saddsub,
	exec_saddsub2,
	exec_saddsu2,
	exec_saddus2,
	exec_saddu4,

	exec_sat,
	exec_set,
	exec_shfl,
	exec_shfl3,
	exec_shl,
	exec_shlmb,
	exec_shr,
	exec_shr2,
	exec_shrmb,

	exec_shru,
	exec_shru,
	exec_shru,
	exec_shru,
	exec_shru2,
	exec_sl,

	exec_smpy,
	exec_smpyh,
	exec_smpyhl,
	exec_smpylh,
	exec_smpy2,
	exec_smpy32,
	exec_spack2,
	exec_spacku4,
	exec_spkernel,
	exec_spkernelr,
	exec_sploop,
	exec_sploopd,
	exec_sploopw,
	exec_spmask,
	exec_spmaskr,
	exec_sshl,
	exec_sshvl,
	exec_sshvr,
	exec_ssub,
	exec_ssub2,

	exec_stb,
	exec_stb_15,
	exec_stdw,
	exec_sth,
	exec_sth_15,
	exec_stndw,
	exec_stnw,
	exec_stw,
	exec_stw_15,

	exec_sub,
	exec_sub,
	exec_sub,
	exec_sub,
	exec_sub,
	exec_sub,
	exec_sub_d_x,
	exec_sub_d,
	exec_sub_d,

	exec_subab,
	exec_subabs4,
	exec_subah,
	exec_subaw,
	exec_subc,
	exec_subu,
	exec_sub2,
	exec_sub4,

	exec_swap2,
	exec_swap4,
	exec_swe,
	exec_swenr,
	exec_unpkhu4,
	exec_unpklu4,
	exec_xor,
	exec_xormpy,
	exec_xpnd2,
	exec_xpnd4,
	exec_zero,
	NULL,
};
uint32_t exec_insn(c6k_core_t* core, uint32_t* fetch_packet){
	/* ABS */
	/* compact instruction format */
	generic_address_t pc = core->pc;
	if((fetch_packet[FP_SIZE - 1] >> 28) == 0xe){
		uint32_t header = fetch_packet[FP_SIZE - 1];
		int layout = (header >> 21) & 0x7f;
		int expansion = (header >> 14) & 0x7f;
		int dsz = (header >> 16) & 0x3;
		int pbits = header & 0x3FFF;
		//DBG("In %s, compact insn executed at 0x%x. header is 0x%x\n", __FUNCTION__, core->pc, fetch_packet[FP_SIZE - 1]);
		//DBG("In %s, compact insn executed at 0x%x. expansion is 0x%x\n", __FUNCTION__, core->pc, expansion);
		//DBG("In %s, compact insn executed at 0x%x. layout is 0x%x\n", __FUNCTION__, core->pc, layout);
		//int pbit = 0, last_pbit = 0;
		int pbit = core->parallel;
		int i = ((core->pc  & 0xFFFFFFFC)- core->pce1) / 4;
		if((core->pc & 0x1f) == 0x1c){
			/* skip header word */
			core->pc += 4;
			return 0;
		}
		//DBG("In %s, i = 0x%x, core->pce1=0x%x, pc=0x%x\n", __FUNCTION__, i, core->pce1, core->pc);
		//while( i < (FP_SIZE - 1)){
		if( i < (FP_SIZE - 1)){
			//DBG("In %s COMPACT, i = 0x%x, core->pce1=0x%x, parallel=%d, pc=0x%x\n", __FUNCTION__, i, core->pce1, core->parallel, core->pc);
			/*
			if(core->spmask && (core->pc >= core->spmask_begin && core->pc <= core->spmask_end)){
				DBG("In %s, skip mask instruction 0x%x, layout=0x%x, i= %d,pc=0x%x\n", __FUNCTION__, fetch_packet[i], layout, i, core->pc);
				if((layout >> i) & 0x1){
					core->pc += 2;
				}
				else
					core->pc += 4;
				DBG("In %s, skip mask instruction 0x%x, pc=0x%x\n", __FUNCTION__, fetch_packet[i], core->pc);
				i = ((core->pc  & 0xFFFFFFFC)- core->pce1) / 4;
				continue;
			}*/

			/* if compact insn */
			if((layout >> i) & 0x1){
				uint32_t insn_0, insn_1;
	
				/* a branch to the second instrucion for the compact layout */
				if((core->pc & 0x2) == 0){
					insn_0 = (fetch_packet[i] & 0xFFFF) | (expansion << 16);
			
					exec_16b_insn(core, insn_0);
					core->insn_num ++;
					/* last pbit  */
					core->parallel = pbit;
					pbit = (pbits >> (i * 2)) & 0x1;
					/**/
					if(core->pc > (core->sploop_begin)){
						record_sploop_buffer(core, fetch_packet[i], (layout >> i) & 0x1);
					}
					if(pbit == 0){
						/* end of parallel, we should write back result */
						write_back(core);
						dec_delay_slot(core);
					}
					print_all_gpr(core);
					if(core->delay_slot == 0 && core->pfc != 0){
						//skyeye_printf_in_color(GREEN, "In %s, branch happened at 0x%x, target=0x%x, core->delay_slot_bak1=%d\n", __FUNCTION__, core->pc, core->pfc, core->delay_slot_bak1);
						core->pc = core->pfc;
				
						if(core->delay_slot_bak1){
							core->pfc = core->pfc_bak1;
							core->delay_slot = core->delay_slot_bak1;
							core->delay_slot_bak1 = 0;
							core->pfc_bak1 = 0xFFFFFFFF;
							DBG("In %s, begin to count next branch,target=0x%x,delay_slot=%d\n", __FUNCTION__, core->pfc, core->delay_slot);
						}
						else
							core->pfc = 0;
						print_all_gpr(core);
						if(((core->pc - core->pce1) / 4) < (FP_SIZE - 1)){
							i = ((core->pc - core->pce1) / 4);
							DBG("In %s, branch triggered inner packet, target=0x%x, i=%d\n", __FUNCTION__, core->pc, i);
							//continue;
							return 0;
						}
						else
							return 0;

					}

				}

				insn_1 = (fetch_packet[i] >> 16) |  (expansion << 16);
				//last_pbit = pbit;
				exec_16b_insn(core, insn_1);
				core->insn_num ++;
				core->parallel = pbit;
				pbit = (pbits >> (i * 2)) & 0x2;

				if(pbit == 0){
					write_back(core);
					dec_delay_slot(core);
				}

				print_all_gpr(core);
			}
			else{
				//DBG("In %s, instr=0x%x at 0x%x\n", __FUNCTION__, fetch_packet[i], core->pc);
				//last_pbit = pbit;
				exec_32b_insn(core, fetch_packet[i]);
				core->insn_num ++;
				core->parallel = pbit;
				pbit = fetch_packet[i] & 0x1;
				if(pbit == 0){
					/* end of parallel, we should write back result */
					write_back(core);
					dec_delay_slot(core);
				}
				print_all_gpr(core);
			}
			if(core->pc > (core->sploop_begin)){
				record_sploop_buffer(core, fetch_packet[i], (layout >> i) & 0x1);
				#if 0
				//if((fetch_packet[i -1] & 0x1) == 0){
				//if(core->spmask && (core->pc >= core->spmask_begin && core->pc <= (core->spmask_end + 4))){
				if(core->spmask && (core->pc >= core->spmask_begin && core->pc <= (core->spmask_end))){
					DBG("mask instruction 0x%x, pc=0x%x\n", fetch_packet[i], core->pc);
				}
				else{	
					int pos = core->buffer_pos;
					if((layout >> i) & 0x1){
						if((core->pc & 0x2) == 0)
							core->sploop_buffer[pos] = fetch_packet[i] & 0xFFFF;
						else
							core->sploop_buffer[pos] = fetch_packet[i] >> 16;
					}
					else{	
						core->sploop_buffer[pos] = fetch_packet[i];
					}
					core->buffer_pos++;
					skyeye_printf_in_color(PURPLE, "In %s, record instr at sploop_buffer %d, instr=0x%x\n", __FUNCTION__, core->buffer_pos, core->sploop_buffer[pos]);
					skyeye_printf_in_color(PURPLE, "In %s, pc=0x%x, spmask_begin=0x%x, spmask_end=0x%x, record instr at sploop_buffer\n", __FUNCTION__, core->pc, core->spmask_begin, core->spmask_end);
				}
				#endif
			}
			if(core->sploop_end != 0xFFFFFFFF && (pbit == 0)){
				uint32_t orig_pc = core->pc;
				if(core->sploopw_flag){
					DBG("In %s, begin sploopw, spoolpw_cond=0x%x, buffer_pos=%d, A0=0x%x\n", __FUNCTION__, core->sploopw_cond,core->buffer_pos, core->gpr[GPR_A][0]);
					while(calc_cond(core, core->sploopw_cond)){
						int i = 0;
						for(; i < core->buffer_pos; i++){
							if(core->sploop_buffer[i] >> 16)
								exec_32b_insn(core, core->sploop_buffer[i]);
							else
								exec_16b_insn(core, core->sploop_buffer[i]);
							/* FIXME, should judge the parallel */
							/* end of parallel, we should write back result */
							write_back(core);
							DBG("In %s, in sploopw, i=%d, insn=0x%x\n\n", __FUNCTION__, i, core->sploop_buffer[i]);
						}
					}
					DBG("In %s, end sploopw, spoolpw_cond=0x%x, A0=0x%x\n", __FUNCTION__, core->sploopw_cond, core->gpr[GPR_A][0]);
					core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
					core->buffer_pos = 0;
					core->pc = orig_pc;
					core->sploopw_cond = 0;
					core->spmask = 0;
					core->spmask_begin = 0xFFFFFFFF;
					core->sploopw_flag = 0;
				}
				if(core->sploop_flag && core->buffer_pos){
					DBG("In %s, begin sploop, buffer_pos=%d\n", __FUNCTION__, core->buffer_pos);
					while(core->ilc){
						int i = 0;
						for(; i < core->buffer_pos; i++){
							if(core->sploop_buffer[i] >> 16)
								exec_32b_insn(core, core->sploop_buffer[i]);
							else
								exec_16b_insn(core, core->sploop_buffer[i]);
							/* FIXME, should judge the parallel */
							/* end of parallel, we should write back result */
							write_back(core);
							DBG("In %s, in sploop, i=%d, insn=0x%x, b5=0x%x\n\n", __FUNCTION__, i, core->sploop_buffer[i], core->gpr[GPR_B][5]);
						}

					 /* FIXME, should judge the parallel */
                                                /* end of parallel, we should write back result */
						//write_back(core);
					}

					DBG("In %s, end sploop \n", __FUNCTION__);
					core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
					core->buffer_pos = 0;
					core->pc = orig_pc;
					core->spmask = 0;
					core->spmask_begin = 0xFFFFFFFF;
					core->sploop_flag = 0;
				}

				//sleep(10);
			}
			i++;
			if(core->delay_slot == 0 && core->pfc != 0){
				//skyeye_printf_in_color(GREEN, "In %s, branch happened at 0x%x, target=0x%x, core->delay_slot_bak1=%d\n", __FUNCTION__, core->pc, core->pfc, core->delay_slot_bak1);
				core->pc = core->pfc;
				
				if(core->delay_slot_bak1){
					core->pfc = core->pfc_bak1;
					core->delay_slot = core->delay_slot_bak1;
					core->delay_slot_bak1 = 0;
					core->pfc_bak1 = 0xFFFFFFFF;
					DBG("In %s, begin to count next branch,target=0x%x,delay_slot=%d\n", __FUNCTION__, core->pfc, core->delay_slot);
				}
				else
					core->pfc = 0;
				print_all_gpr(core);
				if(((core->pc - core->pce1) / 4) < (FP_SIZE - 1)){
					i = ((core->pc - core->pce1) / 4);
					DBG("In %s, branch triggered inner packet, target=0x%x, i=%d\n", __FUNCTION__, core->pc, i);
				}
				else
					return 0;

			}

			/* if we reach new packet region */
			if((core->pc & 0x1f) == 0)
				return 0;

		}
		/* skip header word */
		//core->pc += 4;
	}
	else{
		int pbit;
		int i = (core->pc - core->pce1) / 4;
		//while(i < FP_SIZE){
		if(i < FP_SIZE){
			exec_32b_insn(core, fetch_packet[i]);
			core->insn_num ++;
			pbit = fetch_packet[i] & 0x1;	
			core->parallel = pbit;
			if(pbit == 0){
				/* end of parallel, we should write back result */
				write_back(core);
				dec_delay_slot(core);
			}
			if(core->pc > (core->sploop_begin)){
				//if((fetch_packet[i -1] & 0x1) == 0){
					int pos = core->buffer_pos;
					core->sploop_buffer[pos] = fetch_packet[i];
					core->buffer_pos++;
					DBG("In %s, record instr at sploop_buffer, instr=0x%x\n", __FUNCTION__, fetch_packet[i]);
				//}
			}
			if(core->sploop_end != 0xFFFFFFFF && (pbit == 0)){
				uint32_t orig_pc = core->pc;
				while(core->ilc){
					int i = 0;
					for(; i < core->buffer_pos; i++)
						exec_32b_insn(core, core->sploop_buffer[i]);
					 /* FIXME, should judge the parallel */
                                                /* end of parallel, we should write back result */
					write_back(core);
				}
				core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
				core->buffer_pos = 0;
				core->pc = orig_pc;
			}
			/*
			if(core->sploop_end != 0xFFFFFFFF && (pbit == 0)){
				uint32_t orig_pc = core->pc;
				while(core->ilc){
					int i = 0;
					for(; i < core->buffer_pos; i++)
						exec_32b_insn(core, core->sploop_buffer[i]);
				}
				core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
				core->buffer_pos = 0;
				core->pc = orig_pc;
			}*/
			//DBG("In %s, instr=0x%x\n", __FUNCTION__, fetch_packet[i]);
			i++;
			if(core->delay_slot == 0 && core->pfc != 0){
				//skyeye_printf_in_color(GREEN, "In %s, branch happened at 0x%x, target=0x%x\n", __FUNCTION__, core->pc, core->pfc);
				core->pc = core->pfc;
				if(core->delay_slot_bak1){
					core->pfc = core->pfc_bak1;
					core->delay_slot = core->delay_slot_bak1;
					core->delay_slot_bak1 = 0;
					core->pfc_bak1 = 0xFFFFFFFF;
					DBG("In %s, begin to count next branch,target=0x%x,delay_slot=%d\n", __FUNCTION__, core->pfc, core->delay_slot);
				}
				else
					core->pfc = 0;

				/* The branch sometimes will triggered in sploop, so we have to reset the sploop buffer */
				core->sploop_begin = core->sploop_end = 0xFFFFFFFF;
				core->buffer_pos = 0;

				if(((core->pc - core->pce1) / 4) < (FP_SIZE)){
					i = ((core->pc - core->pce1) / 4);
					DBG("In %s, branch triggered inner packet, target=0x%x, i=%d\n", __FUNCTION__, core->pc, i);
				}
				else
					return 0;
				print_all_gpr(core);
			}
			print_all_gpr(core);
			/* if we reach new packet region */
			if((core->pc & 0x1f) == 0)
				return 0;
		}
	}
	return 0;
}
int exec_32b_insn(c6k_core_t* core, uint32_t insn){
	int index;
	int ret;

	if((ret = decode_instr(insn, &index, insn32_decode_table, sizeof(insn32_decode_table)/sizeof(ISEITEM))) == DECODE_SUCCESS){
		//insn_decode_table[index].action(core,instr);
		DBG("In %s, instr=0x%x, index=0x%x\n", __FUNCTION__, insn, index);
		insn_action[index](core, insn);
	}
	else{
		//skyeye_log();
		DBG("In %s, decode error for 0x%x at 0x%x\n", __FUNCTION__, insn, core->pc);
		exit(-1);
	}
	//print_all_gpr(core);
	return ret;
}
int decode_instr(uint32_t insn, int32_t *idx, ISEITEM* table, int table_len)
{
	int n = 0;
	int base = 0;
	int ret = DECODE_FAILURE;
	int i = 0;
	//int instr_slots = sizeof(insn_decode_table)/sizeof(ISEITEM);
	int instr_slots = table_len;
	for (i = 0; i < instr_slots; i++)
	{
//		ret = DECODE_SUCCESS;
		ISEITEM* slot = &table[i];
		n = slot->attribute_value;

		/* if not content, we ignore */
		if(n == 0)
			continue;
		base = 0;
		while (n) {
			if (BITS(slot->content[base], slot->content[base + 1]) != slot->content[base + 2]) {
				break;
			}
			base += 3;
			n --;
		}
		//All conditions is satisfied.
		if (n == 0)
			ret = DECODE_SUCCESS;

		if (ret == DECODE_SUCCESS) {
			*idx = i;
			return ret;
		}
	}
	return ret;
}
