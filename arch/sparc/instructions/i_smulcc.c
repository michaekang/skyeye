/*
 * =====================================================================================
 *
 *       Filename:  i_smulcc.c
 *
 *    Description:  Implementation of the SMULCC instruction
 *
 *       Compiler:  gcc
 *
 *         Author:  David Yu keweihk@gmail.com
 *        Company:  Tsinghua skyeye team
 *
 * =====================================================================================
 */

/*  Format (3)

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

31-30  29-25   24-19     18-14    13            12-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=1 |         simm13           |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 00000
op3             = 011011

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 011011|   rs1   | 1/0 |      simm13/rs2          |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rd, rs1, rs2, imm;

#define SMULCC_CYCLES    1
#define SMULCC_CODE_MASK 0x80d80000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x1b

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_smulcc = {
	execute,
	disassemble,
	SMULCC_CODE_MASK,
	PRIVILEDGE,
	OP,
};



static int execute(void *state)
{
	int64 x, y, z;

	x = (int64)REG(rs1);
	if( imm < 0 )
	{
		y = (int64)REG(rs2);
		print_inst_RS_RS_RD("smulcc",  rs1, rs2, rd);
	}
	else
	{
		y = (int64)sign_ext13(imm);
		print_inst_RS_IM13_RD("smulcc", rs1, sign_ext13(imm), rd);
	}
	x = (x << 32) >> 32;
	y = (y << 32) >> 32;
	z = x * y;

	REG(rd) = (int)z;
	YREG = (int)(z >> 32);

	clear_icc();
	/*  Negative condition  */
	if( bit(z, 31) )
		psr_set_neg();
	/*  Zero condition  */
	if( (int)z == 0 )
		psr_set_zero();

	PCREG = NPCREG;
	NPCREG += 4;

	// Everyting takes some time
	return SMULCC_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & SMULCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
    {
        rd = bits(instr, RD_OFF_last, RD_OFF_first);
        rs1 = bits(instr, RS1_OFF_last, RS1_OFF_first);
        int i = bit(instr, I_OFF);

        if( i )
        {
            imm = bits(instr, SIMM13_OFF_last, SIMM13_OFF_first);
            rs2 = -1;
        }
        else
        {
            rs2 = bits(instr, RS2_OFF_last, RS2_OFF_first);
            imm = -1;
        }

        return 1;
    }
    return 0;
}

