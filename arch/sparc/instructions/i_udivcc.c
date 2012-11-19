/*
 * =====================================================================================
 *
 *       Filename:  i_udivcc.c
 *
 *    Description:  Implementation of the UDIVCC instruction
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
op3             = 011110

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 011110 |   rs1   | 1/0 |      simm13/rs2          |
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

#define UDIVCC_CYCLES    1
#define UDIVCC_CODE_MASK 0x80f00000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x1e

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12

sparc_instruction_t i_udivcc = {
	execute,
	disassemble,
	UDIVCC_CODE_MASK,
	PRIVILEDGE,
	OP,
};

static int execute(void *state)
{
	uint8 Sm, Dm, Rm;
	uint32 x, y;
	uint64 z, dividend = 0;

	x = REG(rs1);
	dividend = YREG;
	dividend = (dividend << 32) | REG(rs1);

	if( imm < 0 )
	{
		if( REG(rs2) == 0 ){
			traps->signal(DIV_BY_ZERO);
			return UDIVCC_CYCLES;
		}
		else
		{
			y = REG(rs2);
			z = (dividend / REG(rs2));
			REG(rd) = z;
			print_inst_RS_RS_RD("udivcc", rs1, rs2, rd);
		}
	}
	else
	{
		if( imm == 0 )
		{
			traps->signal(DIV_BY_ZERO);
			return UDIVCC_CYCLES;
		}
		else
		{
			y = imm;
			z = (dividend / sign_ext13(imm));
			REG(rd) = z;
			print_inst_RS_IM13_RD("udivcc", rs1, sign_ext13(imm), rd);
		}
	}
	Sm = bit(x, 31);
	Dm = bit(y, 31);
	Rm = bit(z, 31);

	clear_icc();

	/*  Set OVERFLOW    */
	//if((Sm && Dm && !Rm) || (!Sm && !Dm && Rm))
	if(z >> 31){
		REG(rd) = 0xffffffff;
		psr_set_overflow();
	}

	/*  ZERO condition  */
	if(z == 0)
		psr_set_zero();
	/*  NEGATIVE condition  */
	if(bit(z, 31))
		psr_set_neg();


	PCREG = NPCREG;
	NPCREG += 4;

	// Everyting takes some time
	return UDIVCC_CYCLES;

}

static int disassemble(uint32 instr, void *state)
{
	int op3 = 0, op;

	op = bits(instr, OP_OFF_last, OP_OFF_first);
	op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

	if( (instr & UDIVCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
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

