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
* @file c6k_decode_16.c
* @brief The decoder of 16 bit instruction
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-07-31
*/

#include "c6k_cpu.h"
#include "c6k_decode.h"
#include <stdio.h>
#include <stdlib.h>
#include "portable/portable.h"
#include "skyeye_types.h"
#include "skyeye_bus.h"
#include "skyeye_uart_ops.h"
#include "bank_defs.h"

#define DEBUG
#include <skyeye_log.h>

#ifdef DBG
//#undef DBG
//#define DBG
//#define DBG(fmt, ...) do { fprintf(stderr, fmt, ## __VA_ARGS__); } while (0)
#endif

extern int decode_instr(uint32_t insn, int32_t *idx, ISEITEM* table, int table_len);
extern void write_buffer(c6k_core_t* core, int regno, uint32_t result);
#define NOT_IMP printf("In %s : line %d, not implement at 0x%x\n", __FUNCTION__, __LINE__, core->pc);exit(-1)
/* 16 - 18 should be DSZ */
const ISEITEM insn16_decode_table[] = {
{"doff4", 2, 6, 1, 2, 0x2, 10, 10, 0x0},
{"doff4_dw", 3, 6, 1, 2, 0x2, 9, 10, 0x0, 18, 18, 0x1},
{"dind", 2, 6, 1, 2, 0x2, 10, 11, 0x1},
{"dind_dw", 3, 6, 1, 2, 0x2, 9, 11, 0x2, 18, 18, 0x1},
{"dinc", 3, 6, 1, 2, 0x2, 10, 11, 0x3, 14, 15, 0x0},
{"dinc_dw", 4, 6, 1, 2, 0x2, 9, 11, 0x6, 14, 15, 0x0, 18, 18, 0x1},
{"ddec", 3, 6, 1, 2, 0x2, 10, 11, 0x3, 14, 15, 0x1},
{"ddec_dw", 4, 6, 1, 2, 0x2, 9, 11, 0x6, 14, 15, 0x1, 18, 18, 0x1},
{"dstk", 3, 6, 1, 2, 0x2, 10, 11, 0x3, 15, 15, 0x1},
{"dx2op", 2, 6, 1, 6, 0x1b, 10, 10, 0x0},
{"dx5", 2, 6, 1, 6, 0x1b, 10, 10, 0x1},
{"dx5p", 2, 6, 1, 6, 0x3b, 10, 12, 0x3},
{"dx1", 2, 6, 1, 6, 0x3b, 10, 12, 0x6},
{"dpp", 2, 6, 0, 6, 0x77, 11, 11, 0x0},
{"l3", 2, 6, 1, 3, 0x0, 10, 10, 0x0},
{"l3i", 2, 6, 1, 3, 0x0, 10, 10, 0x1},
{"ltbd", 2, 6, 1, 3, 0x4, 10, 10, 0x0},
{"l2c", 2, 6, 1, 3, 0x4, 10, 10, 0x1},
{"lx5", 2, 6, 1, 6, 0x13, 10, 10, 0x1},
{"lx3c", 3, 6, 1, 6, 0x13, 10, 10, 0x0, 12, 12, 0x0},
{"lx1c", 3, 6, 1, 6, 0x13, 10, 10, 0x0, 12, 12, 0x1},
{"lx1", 3, 6, 1, 6, 0x33, 10, 12, 0x6},
{"m3", 1, 6, 1, 4, 0xf},
{"sbs7", 1, 6, 1, 5, 0x5},
{"sbu8", 2, 6, 1, 5, 0x5, 14, 15, 3},
{"scs10", 1, 6, 1, 5, 0xd},
{"sbs7c", 2, 6, 1, 3, 0x5, 5, 5, 0x1},
{"sbu8c", 2, 6, 1, 3, 0x5, 5, 5, 0x1, 14, 15, 0x3},
{"s3", 2, 6, 1, 3, 0x5, 10, 10, 0x0},
{"s3i", 2, 6, 1, 3, 0x5, 10, 10, 0x1},
{"smvk8", 1, 6, 1, 4, 0x9},
{"ssh5", 2, 6, 1, 4, 0x1, 10, 10, 0x1},
{"s2sh", 2, 6, 1, 6, 0x31, 10, 10, 0x1},
{"sc5", 2, 6, 1, 4, 0x1, 10, 10, 0x0},
{"s2ext", 2, 6, 1, 6, 0x31, 10, 10, 0x0},
{"sx2op", 2, 6, 1, 6, 0x17, 10, 10, 0x0},
{"sx5", 2, 6, 1, 6, 0x17, 10, 10, 0x1},
{"sx1", 2, 6, 1, 6, 0x37, 10, 12, 0x6},
{"sx1b", 2, 6, 1, 6, 0x37, 11, 12, 0x0},
{"lsd_mvto", 2, 6, 1, 2, 0x3, 5, 6, 0x0},
{"lsd_mvfr", 2, 6, 1, 2, 0x3, 5, 6, 0x2},
{"lsd_x1c", 2, 6, 1, 2, 0x3, 10, 12, 0x2},
{"lsd_x1", 3, 6, 1, 2, 0x3, 5, 6, 0x3, 10, 12, 0x6},
{"uspl", 3, 6, 1, 6, 0x33, 10, 13, 0x3, 15, 15, 0x0},
{"uspldr", 3, 6, 1, 6, 0x33, 10, 13, 0x3, 15, 15, 0x1},
{"uspk", 2, 6, 1, 6, 0x33, 10, 13, 0x7},
{"uspm", 2, 6, 1, 6, 0x33, 10, 13, 0xb},
{"uspm", 2, 6, 1, 6, 0x33, 10, 13, 0xf},
{"unop", 1, 6, 0, 12, 0xc6e},

};

int get_func_unit(c6k_core_t* core, uint32_t insn){
	if(((insn >> 1) & 0x3) == 0x3 && ((insn >> 5) & 0x3) == 0x2){
		int unit = (insn >> 3) & 0x3;
		int s = BITS(0, 0);
		if(unit == 0){
			return 1 << (L1_UNIT + s);
		}
		else if(unit == 1)
			return 1 << (S1_UNIT + s);
		else if(unit == 2)
			return 1 << (D1_UNIT + s);
		else{
			NOT_IMP;
		}
	}
	else{
		NOT_IMP;
	}
	return 0;
}
static int exec_doff4(c6k_core_t* core, uint32_t insn){
	int ld_st = BITS(3, 3);
	int sz = BITS(9, 9);
	int src = BITS(4, 6);
	int ucst3 = BITS(11, 11);
	int ucst_0_2 = BITS(13, 15); 
	int ptr = BITS(7, 8);
	int s = BITS(0, 0);
	int t = BITS(12, 12);
	int dsz = DSZ(core->header);
	if(ld_st){
		if(sz){
			if(dsz == 0){ /* ldbu */
				int base = ptr | 0x4;
				int ucst = (ucst3 << 3) | ucst_0_2;
				generic_address_t addr = core->gpr[s][base] + ucst;
				bus_read(8, addr, &core->gpr[t][src]);
				DBG("In %s, ldbu addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
			}
			else if(dsz == 0x1 || dsz == 0x5){
				/* ldb */
				int base = ptr | 0x4;
				int ucst = (ucst3 << 3) | ucst_0_2;
				generic_address_t addr = core->gpr[s][base] + ucst;
				bus_read(8, addr, &core->gpr[t][src]);
				/* FIXME, sign extend is needed here */
				DBG("In %s, ldbu addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);

			}

			else{
				NOT_IMP;
			}
		}
		else{
			int base = ptr | 0x4;
			int ucst = (ucst3 << 3) | ucst_0_2;
			generic_address_t addr = core->gpr[s][base] + ucst;
			bus_read(32, addr, &core->gpr[t][src]);
			DBG("In %s, stw addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
			DBG("In %s, header=0x%x, dsz=0x%x\n", __FUNCTION__, core->header, dsz);
			//sleep(10);
		}

	}
	else{
		if(sz){
			if((dsz == 0x0) || (dsz == 0x1) || (dsz == 0x5)){
				/* STB */
				int base = ptr | 0x4;
				int ucst = (ucst3 << 3) | ucst_0_2;
				generic_address_t addr = core->gpr[s][base] + ucst;
				bus_write(8, addr, core->gpr[t][src]);
				DBG("In %s, stb header=0x%x, dsz=0x%x\n", __FUNCTION__, core->header, dsz);
				DBG("In %s, stb addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
				//printf("In %s, stb addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
				//printf("In %s, stb addr=0x%x, src=%d, base=%d, insn=0x%x, val=0x%x\n", __FUNCTION__, addr, src, base, insn, core->gpr[t][src]);
				if(addr >= 0x817ded && addr < (0x817ded + 0x10)){
					char c = core->gpr[t][src] & 0xFF;
					//skyeye_uart_write(0, &c, 1, NULL);
				}

				//sleep(10);
			}
			else{
				NOT_IMP;
			}
		}
		else{
			/* STW */
			//NOT_IMP;
			int base = ptr | 0x4;
			int ucst = (ucst3 << 3) | ucst_0_2;
			generic_address_t addr = core->gpr[s][base] + ucst;
			bus_write(32, addr, core->gpr[t][src]);
			DBG("In %s, addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
			//sleep(10);
		}
	}
	core->pc += 2;
	return 0;
}
static int exec_doff4_dw(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dind(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dind_dw(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dinc(c6k_core_t* core, uint32_t insn){
	int ld_st = BITS(3, 3);
	int sz = BITS(9, 9);
	int src = BITS(4, 6);
	int ucst0 = BITS(13, 13); 
	int ptr = BITS(7, 8);
	int s = BITS(0, 0);
	int t = BITS(12, 12);
	int dsz = (core->header >> 16) & 0x7;
	int base = ptr | 0x4;
	if(ld_st){
		if(sz){
			if(dsz == 0){
				int ucst2 = ucst0 + 1;
				generic_address_t addr = core->gpr[s][base];
				core->gpr[s][base] += ucst2;
				DBG("In %s, addr=0x%x, base=0x%x, ucst2=%d, insn=0x%x\n", __FUNCTION__, addr, core->gpr[s][base], ucst2, insn);
				bus_read(8, addr, &core->gpr[t][src]);
			}
			else{
				NOT_IMP;
			}
		}
		else{
			NOT_IMP;
		}
	}
	else{
		if(sz){
			if((dsz == 0) || (dsz == 0x1) || (dsz == 0x5)){

				int ucst2 = ucst0 + 1;
				generic_address_t addr = core->gpr[s][base];
				core->gpr[s][base] += ucst2;
				bus_write(8, addr, core->gpr[t][src]);
				DBG("In %s, stb, base=0x%x, addr=0x%x\n", __FUNCTION__, core->gpr[s][base], addr);
				//sleep(10);
			}
			else{
				NOT_IMP;
			}

		}
		else{
			int na = BITS(4, 4);
			if(dsz & 0x4){
				if(na == 0){ /* stdw */
					int ucst2 = ucst0 + 1;
					generic_address_t addr = core->gpr[s][base];
					core->gpr[s][base] += (ucst2 << 3) ;
					bus_write(32, addr, core->gpr[t][src]);
					bus_write(32, addr + 4, core->gpr[t][src + 1]);

				}
				else{
					NOT_IMP;
				}
			}
			else{
				DBG("In %s, dsz = %d, header=0x%x, header >> 15=0x%x\n", __FUNCTION__, dsz, core->header, core->header >> 16);
				NOT_IMP;
			}
		}
	}
	core->pc += 2;
	return 0;
}
static int exec_dinc_dw(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_ddec(c6k_core_t* core, uint32_t insn){
	int ld_st = BITS(3, 3);
	int sz = BITS(9, 9);
	int src = BITS(4, 6);
	int ucst = BITS(13, 13) + 1;
	int ptr = BITS(7, 8);
	int s = BITS(0, 0);
	int t = BITS(12, 12);
	int dsz_2 = BITS(20, 20);
	if(ld_st){
		NOT_IMP;
	}
	else{
		if(sz){
			NOT_IMP;
		}
		else{
			NOT_IMP;
			/* STW */
			int base = ptr | 0x4;
			core->gpr[s][base] -= ucst;
			generic_address_t addr = core->gpr[s][base];
			bus_write(32, addr, core->gpr[t][src]);
			DBG("In %s, addr=0x%x, src=%d, base=%d, insn=0x%x\n", __FUNCTION__, addr, src, base, insn);
			//sleep(10);
		}
	}
	core->pc += 2;
	return 0;
}
static int exec_ddec_dw(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dstk(c6k_core_t* core, uint32_t insn){
	int ld_st = BITS(3, 3);
	int t = BITS(12, 12);
	int src = BITS(4, 6);
	int ucst5 = BITS(13, 14) | (BITS(7, 9) << 2);
	if(ld_st){
		//NOT_IMP;
		/* ldw */
		generic_address_t addr = core->gpr[GPR_B][15] + (ucst5 << 2);
		bus_read(32, addr, &core->gpr[t][src]);
		DBG("In %s, LDW@0x%x, addr=0x%x, src=%d, ucst5\n", __FUNCTION__, core->pc, addr, src, ucst5);
		DBG("In %s, LDW@0x%x, addr=0x%x, src=%d, ucst5\n", __FUNCTION__, core->pc, addr, src, ucst5);

	}
	else{
		/* stw */
		generic_address_t addr = core->gpr[GPR_B][15] + (ucst5 << 2);
		bus_write(32, addr, core->gpr[t][src]);
		DBG("In %s, STW@0x%x, addr=0x%x, src=%d, ucst5\n", __FUNCTION__, core->pc, addr, src, ucst5);
	}
	core->pc += 2;
	return 0;
}
static int exec_dx2op(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dx5(c6k_core_t* core, uint32_t insn){
	int ucst2_0 = BITS(13, 15);
	int ucst3_4 = BITS(11, 12);
	int ucst5 = (ucst3_4 << 3) | ucst2_0;
	int dst = BITS(7, 9);
	int s = BITS(0, 0);
	core->gpr[s][dst] = core->gpr[GPR_B][15] + (ucst5 << 2);
	core->pc += 2;
	return 0;
}
static int exec_dx5p(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_dx1(c6k_core_t* core, uint32_t insn){
	int op = BITS(13, 15);
	int src_dst = BITS(7, 9);
	int s = BITS(0, 0);
	if(op == 0x0){
		core->gpr[s][src_dst] = 0;
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_dpp(c6k_core_t* core, uint32_t insn){
	int dw = BITS(15, 15);
	int ld_st = BITS(14, 14);
	int t = BITS(12, 12);
	int src_dst = BITS(7, 10);
	unsigned int ucst0 = BITS(13, 13);
	generic_address_t addr;
	if(ld_st){ /* load */
		if(dw){
			//NOT_IMP;
			/* lddw ++B15 */
			core->gpr[GPR_B][15] = core->gpr[GPR_B][15] + ((ucst0 + 1) << 3);
			//core->gpr[GPR_B][15] = core->gpr[GPR_B][15] + ((ucst0 + 1) << 2);
			addr = core->gpr[GPR_B][15];
			bus_read(32, addr, &core->gpr[t][src_dst]);
			bus_read(32, addr + 4, &core->gpr[t][src_dst + 1]);
			//sleep(10);
			DBG("In %s, lddw, addr=0x%x\n", __FUNCTION__, addr);

		}
		else{
			/* ldw ++b15 */
			core->gpr[GPR_B][15] = core->gpr[GPR_B][15] + ((ucst0 + 1) << 2);
			addr = core->gpr[GPR_B][15];
			bus_read(32, addr, &core->gpr[t][src_dst]);

		}
	}
	else{
		/* store */
		if(dw){
			/* double word */
			//NOT_IMP;
			/* stdw *B15--[ucst2] */
			addr = core->gpr[GPR_B][15];
			core->gpr[GPR_B][15] = core->gpr[GPR_B][15] - ((ucst0 + 1) << 3);
			bus_write(32, addr, core->gpr[t][src_dst]);
			//core->gpr[GPR_B][15] = core->gpr[GPR_B][15] - ((ucst0 + 1) << 2);
			bus_write(32, addr + 4, core->gpr[t][src_dst + 1]);
		}
		else{
			//if(core->pc == 0x810ac0){
			if(0){
				DBG("In %s, the instruction is inhibited at 0x%x.\n", __FUNCTION__, core->pc);
			}
			else{
				/* stw *B15--[ucst2] */
				addr = core->gpr[GPR_B][15];
				core->gpr[GPR_B][15] = core->gpr[GPR_B][15] - ((ucst0 + 1) << 2);
				bus_write(32, addr, core->gpr[t][src_dst]);
				DBG("In %s, addr=0x%x, src_dst=%d, ucst0=%d\n", __FUNCTION__, addr, src_dst, ucst0);
			}
		}
	}
	DBG("In %s, src_dst=%d\n", __FUNCTION__, src_dst);
	//sleep(5);
	core->pc += 2;
	return 0;
}
static int exec_l3(c6k_core_t* core, uint32_t insn){
	int op = BITS(11, 11);
	int src1 = BITS(13, 15);
	int src2 = BITS(7, 9);
	int x = BITS(12, 12);
	int dst = BITS(4, 6);
	int s = BITS(0, 0);
	int sat = (core->header >> 14) & 0x1;
	if((op == 1) && (sat == 0)){
		/* SUB */
		if(x)
			core->gpr[s][dst] = core->gpr[s][src1] - core->gpr[(!s) & 0x1][src2];
		else
			core->gpr[s][dst] = core->gpr[s][src1] - core->gpr[s][src2];
		DBG("In %s, src1=%d, src2=%d, dst=%d\n", __FUNCTION__, src1, src2, dst);
	}
	else if((op == 0) && (sat == 0)){
		/* ADD */
		if(x)
			core->gpr[s][dst] = core->gpr[s][src1] + core->gpr[(!s) & 0x1][src2];
		else
			core->gpr[s][dst] = core->gpr[s][src1] + core->gpr[s][src2];
		DBG("In %s, src1=%d, src2=%d, dst=%d\n", __FUNCTION__, src1, src2, dst);
		printf("In %s, src1=%d, src2=%d, dst=%d\n", __FUNCTION__, src1, src2, dst);

	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	//sleep(3);
	return 0;
}

static int exec_l3i(c6k_core_t* core, uint32_t insn){
	int sn = BITS(11, 11);
	int cst3 = BITS(13, 15);
	int dst = BITS(4, 6);
	int src2 = BITS(7, 9);
	int x = BITS(12, 12);
	int s = BITS(0, 0);
	int scst5;
	if(sn){
		scst5 = cst3 | 0x18;
		scst5 = SIGN_EXTEND(scst5, 5);
	}
	else{
		if(cst3 == 0)
			scst5 = 8;
		else
			scst5 = cst3;
	}
	if(x)
		core->gpr[s][dst] = core->gpr[(!s) & 0x1][src2] + scst5;
	else
		core->gpr[s][dst] = core->gpr[s][src2] + scst5;
	DBG("In %s, scst5=%d\n", __FUNCTION__, scst5);
	core->pc += 2;
	return 0;
}

static int exec_ltbd(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_l2c(c6k_core_t* core, uint32_t insn){
	int op = BITS(5, 6) | (BITS(11, 11) << 2);
	int src1 = BITS(13, 15);
	int src2 = BITS(7, 9);
	int dst = BITS(4, 4);
	int s = BITS(0, 0);
	int x = BITS(12, 12);
	if(op == 3){
		if(x)
			core->gpr[s][dst] = (core->gpr[s][src1] == core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = (core->gpr[s][src1] == core->gpr[s][src2]);
	}
	else if(op == 1){
		if(x)
			core->gpr[s][dst] = (core->gpr[s][src1] | core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = (core->gpr[s][src1] | core->gpr[s][src2]);

	}
	else if(op == 0x0){
		if(x)
			core->gpr[s][dst] = (core->gpr[s][src1] & core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = (core->gpr[s][src1] & core->gpr[s][src2]);
		DBG("In %s, and, src1=%d, src2=%d, dst=%d\n", __FUNCTION__, src1, src2, dst);
		//sleep(3);
	}
	else if(op == 0x5){
		/* CMPGT , FIXME, sign comparison */
		if(x)
			core->gpr[s][dst] = ((int)core->gpr[s][src1] > (int)core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = ((int)core->gpr[s][src1] > (int)core->gpr[s][src2]);

	}
	else if(op == 0x7){
		/* CMPGTU */
		if(x)
			core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = (core->gpr[s][src1] > core->gpr[s][src2]);

	}
	else if(op == 0x6){
		/* CMPLTU */
		if(x)
			core->gpr[s][dst] = (core->gpr[s][src1] < core->gpr[(!s) & 0x1][src2]);
		else
			core->gpr[s][dst] = (core->gpr[s][src1] < core->gpr[s][src2]);
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_lx5(c6k_core_t* core, uint32_t insn){
	int dst = BITS(7, 9);
	int scst5 = BITS(13, 15) | (BITS(11, 12) << 3);
	int s = BITS(0, 0);
	int result = SIGN_EXTEND(scst5, 5);
	write_buffer(core, dst + s * 32, result);
	//NOT_IMP;
	DBG("In %s, dst=%d, scst5=%d\n", __FUNCTION__, dst, SIGN_EXTEND(scst5, 5));
	//sleep(10);
	core->pc += 2;
	return 0;
}
static int exec_lx3c(c6k_core_t* core, uint32_t insn){
	int ucst3 = BITS(13, 15);
	int dst = BITS(11, 11);
	int src2 = BITS(7, 9);
	int s = BITS(0, 0);
	core->gpr[s][dst] = (core->gpr[s][src2] == ucst3);
	DBG("In %s, dst=%d, src2=%d\n", __FUNCTION__, dst, src2);
	core->pc += 2;
	return 0;
}
static int exec_lx1c(c6k_core_t* core, uint32_t insn){
	int op = BITS(14, 15);
	int ucst1 = BITS(13, 13);
	int dst = BITS(11, 11);
	int src2 = BITS(7, 9);
	int s = BITS(0, 0);
	if(op == 1){
		/* CMPGT */
		core->gpr[s][dst] = (ucst1 > core->gpr[s][src2]);
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_lx1(c6k_core_t* core, uint32_t insn){
	int op = BITS(13, 15);
	int src_dst = BITS(7, 9);
	int s = BITS(0, 0);
	if(op == 0x7){
		core->gpr[s][src_dst] = 1 ^ core->gpr[s][src_dst];
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_m3(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_sbs7(c6k_core_t* core, uint32_t insn){
	/* bnop */
	int n3 = BITS(13, 15);
	int scst7 = BITS(6, 12);
	scst7 = SIGN_EXTEND(scst7, 7);
	core->pfc = core->pce1 + (scst7 << 1);
	if(core->delay_slot){
			NOT_IMP;
	}
	
	core->delay_slot = 5 + 1;
	/* nop insert */
	if(core->delay_slot)
		core->delay_slot -= n3;
	DBG("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);
	//printf("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);
	core->pc += 2;
	//NOT_IMP;
	return 0;
}
static int exec_sbu8(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_scs10(c6k_core_t* core, uint32_t insn){
	//NOT_IMP;
	/* callp */
	int scst10 = BITS(6, 15);
	int s = BITS(0, 0);
	//core->pc = core->gpr[s][3];
	if(scst10 & 0x200){
		scst10 |= 0xFFFFFC00;
		scst10 = scst10 << 2;
	}
	generic_address_t orig_pc = core->pc;
	generic_address_t addr = core->pc;
	//core->pc = (signed int)core->pce1 + scst10;
	core->pfc = (signed int)core->pce1 + scst10;
	core->delay_slot = 0;
	DBG("In %s ,scst10=%d, target=0x%x\n", __FUNCTION__, scst10, core->pc);
	/* Fixme , should calculate the parallel */
	//core->gpr[s][3] += 4;
	uint32_t layout = (core->header >> 21) & 0x7f;
	uint32_t pbits = core->header & 0x3FFF;
	int i = (addr - core->pce1) / 2;
	addr += 2;
	if((pbits >> i) & 0x1){ /* parallel */
		if((layout >> ((i + 1)/ 2 )) & 0x1){
			addr += 2;
		}
		else{
			addr += 4;
		}
	}
	else{
	}	
	core->gpr[s][3] = addr;
	DBG("In %s, return addr=0x%x, pc=0x%x, i=%d\n", __FUNCTION__, addr, orig_pc, i);
	core->pc += 2;
	//NOT_IMP;

	return 0;
}
static int exec_sbs7c(c6k_core_t* core, uint32_t insn){
	int s = BITS(0, 0);
	int z = BITS(4, 4);
	int sz = (s << 1) | z;
	int n3 = BITS(13, 15);
	int scst7 = BITS(6, 12);
	int A0 = core->gpr[GPR_A][0];
	int B0 = core->gpr[GPR_B][0];
	/* sign extend */
	scst7 = (scst7 & 0x40) ? (scst7 | 0xFFFFFF80) : scst7;
	switch(sz){
		case 0:
			if(A0){
				/* bnop */
				core->pfc = core->pce1 + (scst7 << 1);
				if(core->delay_slot){
					NOT_IMP;
				}
			
				core->delay_slot = 5 + 1;
				/* nop insert */
				if(core->delay_slot)
					core->delay_slot -= n3;
				DBG("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);
			}
			break;
		case 1:
			if(!A0){
				/* bnop */
				core->pfc = core->pce1 + (scst7 << 1);
				if(core->delay_slot){
					NOT_IMP;
				}

				core->delay_slot = 5 + 1;
				/* nop insert */
				if(core->delay_slot)
					core->delay_slot -= n3;
				DBG("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);

			}
			break;
		case 2:
			if(B0){
				/* bnop */
				core->pfc = core->pce1 + (scst7 << 1);
				if(core->delay_slot){
					NOT_IMP;
				}
			
				core->delay_slot = 5 + 1;
				/* nop insert */
				if(core->delay_slot)
					core->delay_slot -= n3;
				DBG("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);
			}
			break;
		case 3:
			if(!B0){
				/* bnop */
				core->pfc = core->pce1 + (scst7 << 1);
				if(core->delay_slot){
					NOT_IMP;
				}

				core->delay_slot = 5 + 1;
				/* nop insert */
				if(core->delay_slot)
					core->delay_slot -= n3;
				DBG("In %s, bnop, pfc = 0x%x, pc = 0x%x, n3=%d, delay_slot=%d\n", __FUNCTION__, core->pfc, core->pc, n3, core->delay_slot);

			}
			break;

		default:
			NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_sbu8c(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_s3(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_s3i(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}

static int exec_smvk8(c6k_core_t* core, uint32_t insn){
	int dst = BITS(7, 9);
	int ucst5_6 = BITS(5, 6);
	int ucst7 = BITS(10, 10);
	int ucst3_4 = BITS(11, 12);
	int ucst0_2 = BITS(13, 15);
	int ucst8 = ucst0_2 |(ucst3_4 << 3)|(ucst5_6 << 5)|(ucst7 << 7);
	int s = BITS(0, 0);
	core->gpr[s][dst] = ucst8;
	DBG("In %s, ucst8=0x%x, dst=%d\n", __FUNCTION__, ucst8, dst);
	core->pc += 2;
	//NOT_IMP;
	return 0;
}
static int exec_ssh5(c6k_core_t* core, uint32_t insn){
	int op = BITS(5,6);
	int sat = (core->header >> 14) & 0x1;
	int ucst0_2 = BITS(13, 15);
	int ucst3_4 = BITS(11, 12);
	int ucst5 = ucst0_2 | (ucst3_4 << 3);
	int src_dst = BITS(7, 9);
	int s = BITS(0, 0);

	if(sat == 0){
		if(op == 2){
			core->gpr[s][src_dst] >>= ucst5;
		}
		else{
			NOT_IMP;
		}
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_s2sh(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_sc5(c6k_core_t* core, uint32_t insn){
	int op = BITS(5, 6);
	int ucst0_2 = BITS(13, 15);
	int ucst3_4 = BITS(11, 12);
	int src = BITS(7, 9);
	int s = BITS(0, 0);
	int ucst5 = ucst0_2 | (ucst3_4 << 3);
	int v = (0xFFFFFFFF << ucst5) & (0xFFFFFFFF >> (31 - ucst5));
	
	core->gpr[s][src] = core->gpr[s][src] | v;
	DBG("In %s, src=%d, ucst5=%d\n", __FUNCTION__, src, ucst5);

	core->pc += 2;	
	return 0;
}
static int exec_s2ext(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_sx2op(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_sx5(c6k_core_t* core, uint32_t insn){
	int dst= BITS(7, 9);
	int ucst3_4 = BITS(11, 12);
	int ucst0_2 = BITS(13, 15);
	int ucst5 = ucst0_2 | (ucst3_4 << 3);
	int scst5 = SIGN_EXTEND(ucst5, 5);
	int s = BITS(0, 0);
	/* ADDK */
	core->gpr[s][dst] += scst5;
	DBG("In %s , addk, dst=%d, scst5=%d\n", __FUNCTION__, dst, scst5);
	core->pc += 2;
	return 0;
}
static int exec_sx1(c6k_core_t* core, uint32_t insn){
	int op = BITS(13, 15);
	int src = BITS(7, 9);
	int s = BITS(0, 0);
	if(op == 6){
		if(core->ilc == 4)
			core->ilc += core->gpr[s][src];
		else
			core->ilc = core->gpr[s][src];
		DBG("In %s, ilc=%d, core->gpr[s][src]=%d\n", __FUNCTION__, core->ilc, core->gpr[s][src]);
		//sleep(5);
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_sx1b(c6k_core_t* core, uint32_t insn){
	int src2 = BITS(7, 10);
	int n3 = BITS(13, 15);
	int s = BITS(0, 0);
	core->pfc = core->gpr[s][src2];
	if(core->delay_slot){
		NOT_IMP;
	}
		
	core->delay_slot = 5 + 1;
	/* skip the specific nop */
	core->delay_slot -= n3;
	DBG("In %s, pfc=0x%x\n", __FUNCTION__, core->pfc);

	core->pc += 2;
	//NOT_IMP;
	return 0;
}
static int exec_lsd_mvto(c6k_core_t* core, uint32_t insn){
	int unit = BITS(3, 4);
	int dst = BITS(13, 15);
	int srcms = BITS(10, 11);
	int s = BITS(0, 0);
	int src2 = BITS(7, 9);
	int x = BITS(12, 12);

	DBG("In %s, unit=%d\n", __FUNCTION__, unit);
	if(unit == 0){
		int src = (srcms << 3) | src2;
		if(x)
			x = ((!s) & 0x1);	
		else
			x = s;
		core->gpr[s][dst] = core->gpr[x][src];
		DBG("In %s, s=%d, dst=%d,src2=%d\n", __FUNCTION__, s, dst, src);
		//sleep(10);
	}
	else if(unit == 1){
		int src = (srcms << 3) | src2;
		if(x)
			x = ((!s) & 0x1);	
		else
			x = s;
		core->gpr[s][dst] = core->gpr[x][src];
		DBG("In %s, s=%d, dst=%d,src2=%d\n", __FUNCTION__, s, dst, src);
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_lsd_mvfr(c6k_core_t* core, uint32_t insn){
	int unit = BITS(3, 4);
	DBG("In %s, unit=%d\n", __FUNCTION__, unit);
	int dstms = BITS(10, 11);
	int dst = BITS(13, 15);
	int s = BITS(0, 0);
	int src2 = BITS(7, 9);
	int x = BITS(12, 12);

	if(unit == 0x0){
		dst = (dstms << 3) | dst;
		if(x)
			x = ((!s) & 0x1);	
		else
			x = s;
		//core->gpr[s][dst] = core->gpr[x][src2];
		int result = core->gpr[x][src2];
		 /* write the result to WB buffer */
		write_buffer(core, dst + s * 32, result);
		DBG("In %s, s=%d, dst=%d,src2=%d, pc=0x%x\n", __FUNCTION__, s, dst, src2, core->pc);
		//sleep(10);
	}
	else if(unit == 2){
		dst = (dstms << 3) | dst;
		if(x)
			x = ((!s) & 0x1);	
		else
			x = s;
		
		core->gpr[s][dst] = core->gpr[x][src2];
		DBG("In %s, s=%d, dst=%d,src2=%d\n", __FUNCTION__, s, dst, src2);
	}
	else if(unit == 1){
		dst = (dstms << 3) | dst;
		if(x)
			x = ((!s) & 0x1);	
		else
			x = s;
		
		core->gpr[s][dst] = core->gpr[x][src2];
		DBG("In %s, s=%d, dst=%d,src2=%d\n", __FUNCTION__, s, dst, src2);
	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_lsd_x1c(c6k_core_t* core, uint32_t insn){
	int cc = BITS(14, 15);
	int ucst1 = BITS(13, 13);
	int dst = BITS(7, 9);
	int s = BITS(0, 0);
	if(cc == 2){
		if(core->gpr[GPR_B][0]){
			core->gpr[s][dst] = ucst1;
		}
		else{
			/* do nothing */
		}
	}
	else if(cc == 0){
		if(core->gpr[GPR_A][0]){
			core->gpr[s][dst] = ucst1;
		}
		else{
			/* do nothing */
		}

	}
	else if(cc == 0x3){
		if(!core->gpr[GPR_B][0]){
			core->gpr[s][dst] = ucst1;
		}
		else{
			/* do nothing */
		}

	}
	else if(cc = 0x1){
		if(!core->gpr[GPR_A][0]){
			core->gpr[s][dst] = ucst1;
		}
		else{
			/* do nothing */
		}

	}
	else{
		NOT_IMP;
	}
	core->pc += 2;
	return 0;
}
static int exec_lsd_x1(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_uspl(c6k_core_t* core, uint32_t insn){
	int ii = BITS(23, 27);
	int op = BITS(0, 0);
	int layout = (core->header >> 21) & 0x7f ;
	uint32_t pbits = core->header & 0x3FFF;
	generic_address_t addr = core->pc;
	int i = (addr - core->pce1) / 2;
	if((pbits >> i) & 0x1){ /* FIXME, we should calculate the sploop begin */
		if(core->pc == 0x816130)
			core->sploop_begin = core->pc + 0xc;
		else if(core->pc == 0x8163b0)
			core->sploop_begin = core->pc + 0x8;
		else if(core->pc == 0x8163da)
			core->sploop_begin = core->pc + 8;
		else if(core->pc == 0x8141a0)
			core->sploop_begin = core->pc + 8;
		else{
			DBG("sploop maybe wrong for parallel bit at 0x%x\n", core->pc);
			sleep(10);
		}
	}
	else{
		core->sploop_begin = core->pc + 2;
		//core->sploop_end = core->sploop_begin + SPLOOP_BUFFER_SIZE * 4;
	}
	core->sploop_flag = 1;
	DBG("In %s, begin loop at 0x%x, pbits=0x%x, i=%d, parallel=%d, sploop_begin=0x%x\n", __FUNCTION__, core->pc, pbits, i, ((pbits >> i) & 0x1), core->sploop_begin);
	if(core->pc != 0x8163b0){
	}
	if(op) /* for sploopd */
		core->ilc = 4;
	core->pc += 2;

	return 0;
}
static int exec_uspldr(c6k_core_t* core, uint32_t insn){
	NOT_IMP;
	return 0;
}
static int exec_uspk(c6k_core_t* core, uint32_t insn){
	core->sploop_end = core->pc;
	core->ilc --;
	DBG("In %s, sploop_end=0x%x, ilc=%d\n", __FUNCTION__, core->sploop_end, core->ilc);
	core->pc += 2;
	return 0;
}
static int exec_uspm(c6k_core_t* core, uint32_t insn){
	//NOT_IMP;
	
	core->spmask = 1;
	uint32_t addr = core->pc;
	uint32_t word;
	int pbits = 0;
	int layout = 0;
	int i = 0; /* index of one fetch packet */
	layout = (core->header >> 21) & 0x7f;
	pbits = core->header & 0x3FFF;

	i = ((core->pc  & 0xFFFFFFFC)- core->pce1) / 4;

	int parallel;
	if((core->pc & 0x2) == 0)
		parallel = (pbits >> (i * 2)) & 0x1;
	else
		parallel = ((pbits >> (i * 2)) & 0x2) >> 1;
	DBG("In %s, pbits=0x%x, core->header=0x%x, i =%d, parallel=%d\n", __FUNCTION__, pbits, core->header, i, parallel);
	/* next instruction */
	addr += 2;
	i = ((addr & 0xFFFFFFFC)- core->pce1) / 4;
	core->spmask_begin = addr;
	int l1_mask = BITS(0, 0);
	int l2_s1_s2_mask = BITS(7, 9);
	int d1_d2_mask = BITS(14, 15);
	int spmask_unit = 0 | l1_mask | (l2_s1_s2_mask << 1) | (d1_d2_mask << 6);
	DBG("In %s, spmask_unit = 0x%x\n", __FUNCTION__, spmask_unit);
	while(parallel){
		if((addr & 0x2 ) == 0){
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
		}
		else{
			bus_read(32, addr & 0xFFFFFFFC, &word);
		}
		DBG("In %s, layout=0x%x, i=%d, addr=0x%x\n", __FUNCTION__, layout, i, addr);
		if((layout >> (i)) & 0x1){
			int insn_unit = 0;
			int insn_16 = 0;
			if((addr & 0x2) == 0){
				parallel = (pbits >> (i * 2)) & 0x1;
				addr += 2;
				insn_16 = word & 0xFFFF;
				//insn_unit = 0;
				insn_unit = 0xFFFFFFFF;
				if(((insn_16 >> 1) & 0x3) == 0x3 && ((insn_16 >> 5) & 0x3) == 0x2){
					/* FIXME only check the LSDmvfr */
					insn_unit = get_func_unit(core, insn_16);
					DBG("In %s, insn_16=0x%x, insn_unit=0x%x, spmask_unit=0x%x\n", __FUNCTION__, insn_16, insn_unit, spmask_unit);
					//sleep(10);
				}
				//if(parallel == 0 || (insn_unit & spmask_unit) == 0){
				if(((insn_unit & spmask_unit) == 0) && parallel != 0){
					DBG("In %s, not masked, parallel=%d, addr=0x%x, insn_16=0x%x\n", __FUNCTION__, parallel, addr, insn_16);
					//sleep(10);
				}
				if(parallel == 0){
					if((insn_unit & spmask_unit) == 0)
						addr -= 2;

					break;
				}
				insn_unit = 0;
			}
			DBG("In %s, i = %d, pbits=0x%x\n", __FUNCTION__, i, pbits);
			parallel = (pbits >> (i * 2)) & 0x2;
			addr += 2;
			insn_16 = word >> 16;
			insn_unit = 0xFFFFFFFF;
			if(((insn_16 >> 1) & 0x3) == 0x3 && ((insn_16 >> 5) & 0x3) == 0x2){
				/* FIXME only check the LSDmvfr */
				insn_unit = get_func_unit(core, insn_16);
				DBG("In %s, insn_16=0x%x, insn_unit=0x%x, spmask_unit=0x%x, parallel=0x%x\n", __FUNCTION__, insn_16, insn_unit, spmask_unit, parallel);
				//sleep(3);
			}

			if(((insn_unit & spmask_unit) == 0) && parallel != 0){
				DBG("In %s, not masked, parallel=%d, addr=0x%x, insn_16=0x%x\n", __FUNCTION__, parallel, addr, insn_16);
				//sleep(10);
			}
			if(parallel == 0){
				if((insn_unit & spmask_unit) == 0)
					addr -= 2;
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
	core->pc += 2;
	return 0;
}
static int exec_unop(c6k_core_t* core, uint32_t insn){
	int n3 = BITS(13, 15);
	if(core->delay_slot >= n3)
		core->delay_slot -= n3;
	else
		core->delay_slot = 0;
	//NOT_IMP;
	core->pc += 2;
	return 0;
}

static insn_action_t insn16_action[] = {
	exec_doff4,
	exec_doff4_dw,
	exec_dind,
	exec_dind_dw,
	exec_dinc,
	exec_dinc_dw,
	exec_ddec,
	exec_ddec_dw,
	exec_dstk,
	exec_dx2op,
	exec_dx5,
	exec_dx5p,
	exec_dx1,
	exec_dpp,
	exec_l3,
	exec_l3i,
	exec_ltbd,
	exec_l2c,
	exec_lx5,
	exec_lx3c,
	exec_lx1c,
	exec_lx1,
	exec_m3,
	exec_sbs7,
	exec_sbu8,
	exec_scs10,
	exec_sbs7c,
	exec_sbu8c,
	exec_s3,
	exec_s3i,
	exec_smvk8,
	exec_ssh5,
	exec_s2sh,
	exec_sc5,
	exec_s2ext,
	exec_sx2op,
	exec_sx5,
	exec_sx1,
	exec_sx1b,
	exec_lsd_mvto,
	exec_lsd_mvfr,
	exec_lsd_x1c,
	exec_lsd_x1,
	exec_uspl,
	exec_uspldr,
	exec_uspk,
	exec_uspm,
	exec_uspm,
	exec_unop,
	NULL,
};
int exec_16b_insn(c6k_core_t* core, uint32_t insn){
	int index;
	int ret;
	if((ret = decode_instr(insn, &index, insn16_decode_table, sizeof(insn16_decode_table)/sizeof(ISEITEM))) == DECODE_SUCCESS){
		insn16_action[index](core, insn);
	}
	else{
		DBG("In %s, decode error for insn 0x%x\n", __FUNCTION__, insn);
	}
	return ret;
}
