/*
 * =====================================================================================
 *
 *       Filename:  i_fstod.c
 *
 *    Description:  Implementation of the FSTOD instruction
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
| 10 |   rd  | 110100 |   NBZ   |      opf           |    rs2    |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 00000
opf		= 011001001

31-30  29-25   24-19     18-14    	13-5             4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 110100 |   NBZ   |     011001001      |   rs2	 |
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
static int rd, rs1, rs2, fmt;

#define FSTOD_CYCLES    1
#define FSTOD_CODE_MASK 0x81a009c0
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OPF_OFF_first     5
#define OPF_OFF_last      13
#define OPF         0xc9

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define FMT_OFF_first	19
#define FMT_OFF_last	24
#define FMT		0x34


sparc_instruction_t i_fstod = {
    execute,
    disassemble,
    FSTOD_CODE_MASK,
    PRIVILEDGE,
    OP,
};


static int execute(void *state)
{
	/*
	 * va_* are used to save the registers' value
	 */
	s_precision va_rs2;
	d_precision va_rd;
	int cexc = 0;
	/*  Check whether the FPU is enabled or not */
	if( !bit(PSRREG, PSR_EF) )
	{
		traps->signal(FP_DISABLED);
		fsr_set_ftt(unimplemented_FPop);
		return(FSTOD_CYCLES);
	}
	if(rd & 1){
		traps->signal(FP_EXCEPTION);
		fsr_set_ftt(invalid_fp_register);
		return(FSTOD_CYCLES);
	}
	va_rs2.data = fpu_get_single(rs2);

	va_rd.value = va_rs2.value;
	fpu_put_double(va_rd.data, rd);
	print_fpinst_RS_RS("fstod", rs2, rd);

	PCREG = NPCREG;
	NPCREG += 4;

	fsr_clear_ftt();
	// Everyting takes some time
	return FSTOD_CYCLES;

}

static int disassemble(uint32 instr, void *state)
{
	int opf = 0, op;

	op = bits(instr, OP_OFF_last, OP_OFF_first);
	opf = bits(instr, OPF_OFF_last, OPF_OFF_first);
	fmt = bits(instr, FMT_OFF_last, FMT_OFF_first);

	if( (instr & FSTOD_CODE_MASK) && (op == OP) && (opf == OPF) && (fmt == FMT))
	{
		rd = bits(instr, RD_OFF_last, RD_OFF_first);
		rs2 = bits(instr, RS2_OFF_last, RS2_OFF_first);

		return 1;
	}
	return 0;
}

