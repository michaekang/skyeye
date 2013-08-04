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
* @file ti_c64x_regdefs.c
* @brief The register format of ti c64x, refer to ${GDB_SOURCE}/gdb/regformats/tic6x-c64xp.dat
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-02
*/

#include "skyeye2gdb.h"
#include "skyeye_arch.h"
#include "regformat/c6k_regformat.h"

#define NUM_REGS 69

/*
32:A0 32:A1 32:A2 32:A3 32:A4 32:A5 32:A6 32:A7
32:A8 32:A9 32:A10 32:A11 32:A12 32:A13 32:A14 32:A15
32:B0 32:B1 32:B2 32:B3 32:B4 32:B5 32:B6 32:B7 32:B8
32:B9 32:B10 32:B11 32:B12 32:B13 32:B14 32:B15
32:CSR 32:PC
32:A16 32:A17 32:A18 32:A19 32:A20 32:A21 32:A22 32:A23
32:A24 32:A25 32:A26 32:A27 32:A28 32:A29 32:A30 32:A31
32:B16 32:B17 32:B18 32:B19 32:B20 32:B21 32:B22 32:B23
32:B24 32:B25 32:B26 32:B27 32:B28 32:B29 32:B30 32:B31
32:TSR 32:ILC 32:RILC
*/

static int register_raw_size(int x){
	return 4;
}
static int register_byte(int x){
	return (x*4);
}
static int store_register(int rn, unsigned long * memory){
	//bigendSig = state->bigendSig;
	generic_arch_t* arch_instance = get_arch_instance("");
	if(rn >=0 && rn < 16){
		arch_instance->set_regval_by_id(rn, frommem(memory));
	}
	else if(rn == 32)
		arch_instance->set_regval_by_id(CSR_REG, frommem(memory));
	else if(rn == 33)
		arch_instance->set_regval_by_id(PC_REG, frommem(memory));
	else if(rn >= 16 && rn < 32)
		arch_instance->set_regval_by_id(rn - 16 + B0, frommem(memory));
	else if(rn >= 34 && rn < 50)
		arch_instance->set_regval_by_id(rn - 34 + A16, frommem(memory));
	else if(rn >= 50 && rn < 66)
		arch_instance->set_regval_by_id(rn - 50 + B16, frommem(memory));
	else if(rn == 66)
		arch_instance->set_regval_by_id(TSR_REG, frommem(memory));
	else if(rn == 67)
		arch_instance->set_regval_by_id(ILC_REG, frommem(memory));
	else if(rn == 68)
		arch_instance->set_regval_by_id(ILC_REG, frommem(memory));
	else /* FIXME */
		; /* something wrong */

	return 0;
}
static int fetch_register(int rn, unsigned char * memory){
	uint32 regval;
	generic_arch_t* arch_instance = get_arch_instance("");
	int regno;
	if(rn >=0 && rn < 16){
		regno = rn;
	}
	else if(rn == 32)
		regno = CSR_REG;
	else if(rn == 33)
		regno = PC_REG;
	else if(rn >= 16 && rn < 32)
		regno = rn - 16 + B0;
	else if(rn >= 34 && rn < 50)
		regno = rn - 34 + A16;
	else if(rn >= 50 && rn < 66)
		regno = rn - 50 + B16;
	else if(rn == 66)
		regno = TSR_REG;
	else if(rn == 67)
		regno = ILC_REG;
	else if(rn == 68)
		regno = RILC_REG;
	else /* FIXME */
		; /* something wrong */

	regval = arch_instance->get_regval_by_id(regno);
	tomem (memory, regval);
	return 0;
}

/*
 * register arm register type to the array
 */
void init_ti_c64x_register_defs(void){
	static register_defs_t _reg_defs;
	_reg_defs.name = "c6k";
	_reg_defs.register_raw_size = register_raw_size;
	_reg_defs.register_bytes = (NUM_REGS * 4); 	
	_reg_defs.register_byte = register_byte;
	_reg_defs.num_regs = NUM_REGS;
	_reg_defs.max_register_raw_size = 4;
	_reg_defs.store_register = store_register;
	_reg_defs.fetch_register = fetch_register;
	register_reg_type(&_reg_defs);
}
