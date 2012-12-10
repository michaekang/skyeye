/*
 * =====================================================================================
 *
 *       Filename:  i_fcmpd.c
 *
 *    Description:  Implementation of the FCMPD instruction
 *
 *        Version:  1.0
 *        Created:  12/11/08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David.Yu keweihk@gmail.com
 *        Company:  Tsinghua
 *
 * =====================================================================================
 */

/*  Format (3)

31-30  29-25   24-19     18-14         13-5              4-0
+----+-------+--------+------------------------------------------+
| 10 |  abz  | 110101 |   rs1   |      opf           |    rs2    |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 00000
opf		= 001010010

31-30  29-25   24-19     18-14    	13-5             4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 110100 |   rs1   |     001001110      |   rs2	 |
+----+-------+--------+------------------------------------------+

*/
#include "skyeye_fp.h"
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/fpu.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rs1, rs2;

#define FCMPD_CYCLES    1
#define FCMPD_CODE_MASK 0x81a80a40
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OPF_OFF_first    5 
#define OPF_OFF_last     13
#define OPF         0x52

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4


sparc_instruction_t i_fcmpd = {
    execute,
    disassemble,
    FCMPD_CODE_MASK,
    PRIVILEDGE,
    OP,
};


static int execute(void *state)
{
	/*
	 * va_* are used to save the registers' value
	 */
	d_precision va_rs1, va_rs2;
	double result = 0;
	/*  Check whether the FPU is enabled or not */
	if( !bit(PSRREG, PSR_EF) )
	{
		traps->signal(FP_DISABLED);
		fsr_set_ftt(unimplemented_FPop);
		return(FCMPD_CYCLES);
	}
	if((rs1 & 1) || (rs2 & 1)){
		traps->signal(FP_EXCEPTION);
		fsr_set_ftt(invalid_fp_register);
		return(FCMPD_CYCLES);
	}
	va_rs1.data = fpu_get_double(rs1);
	va_rs2.data = fpu_get_double(rs2);
	result = va_rs1.value - va_rs2.value;

	if(result == 0){
		fsr_set_fcc(0);
	}else if(result < 0){
		fsr_set_fcc(1);
	}else if(result > 0){
		fsr_set_fcc(2);
	}else{
		fsr_set_fcc(3);
	}

	print_fpinst_RS_RS("fcmpd", rs1, rs2);
	PCREG = NPCREG;
	NPCREG += 4;

	fsr_clear_ftt();
	// Everyting takes some time
	return FCMPD_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
	int opf = 0, op;

	op = bits(instr, OP_OFF_last, OP_OFF_first);
	opf = bits(instr, OPF_OFF_last, OPF_OFF_first);

	if( (instr & FCMPD_CODE_MASK) && (op == OP) && (opf == OPF) )
	{
		rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);
		rs2 = bits(instr, RS2_OFF_last, RS2_OFF_first);

		return 1;
	}
	return 0;
}

