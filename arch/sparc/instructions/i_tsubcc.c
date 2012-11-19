/*
 * =====================================================================================
 *
 *       Filename:  i_tsubcc.c
 *
 *    Description:  Implementation of the TTSUBCC instruction
 *
 *         Author:  Daivd Yu keweihk@gmail.com
 *        Company:  Tinghua skyeye team
 *
 * =====================================================================================
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

#define TSUBCC_CYCLES    1
#define TSUBCC_CODE_MASK 0x81080000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x21

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12


sparc_instruction_t i_tsubcc = {
	execute,
	disassemble,
	TSUBCC_CODE_MASK,
	PRIVILEDGE,
	OP,
};

static int execute(void *state)
{
	int x, y, z;
	uint8 Sm, Dm, Rm;

	x = REG(rs1);
	if( imm < 0 )
	{
		z = REG(rd) = (REG(rs1) - REG(rs2));
		y = REG(rs2);
		print_inst_RS_RS_RD("tsubcc", rs1, rs2, rd);
	}
	else
	{
		z = REG(rd) = (REG(rs1) - sign_ext13(imm));
		y = sign_ext13(imm);
		print_inst_RS_IM13_RD("tsubcc", rs1, sign_ext13(imm), rd);
	}
	/*
	 * Clear condition codes
	 */
	clear_icc();

	Sm = bit(x, 31);
	Dm = bit(y, 31);
	Rm = bit(z, 31);
	/*  Set the CARRY condition */
	if((!Sm && Dm) || (Rm && (!Sm || Dm))){
		psr_set_carry();
	}
	/*  Negative condition  */
	if(bit(z, 31)){
		psr_set_neg();
	}
	/*  Zero condition  */
	if(z == 0){
		psr_set_zero();
	}
	/*
	 * A tag_overflow occurs if bit 1 or bit 0 of either
	 * operand is nonzero, or if the subtraction generates
	 * an arithmetic overflow
	 */
	if(x & 0x3 || y & 0x3){
		traps->signal(TAG_OVERFLOW);
		psr_set_overflow();
		return TSUBCC_CYCLES;
	}
	if ((Sm != Dm) && (Sm != Rm)){
		traps->signal(TAG_OVERFLOW);
		psr_set_overflow();
		return TSUBCC_CYCLES;
	}

	PCREG = NPCREG;
	NPCREG += 4;
	/* Everyting takes some time */
	return(TSUBCC_CYCLES);
}

/*  Format

    31-30  29-25   24-19     18-14    13        12-5          4-0
    +----+-------+--------+------------------------------------------+
    | op |   rd  |  op3   |   rs1   | i=0 |   non-used   |    rs2    |
    +----+-------+--------+------------------------------------------+

    31-30  29-25   24-19     18-14    13            12-0
    +----+-------+--------+------------------------------------------+
    | op |   rd  |  op3   |   rs1   | i=1 |         simm13           |
    +----+-------+--------+------------------------------------------+

    op              = 10
    rd              = 0000
    op3             = 010100

    31-30  29-25   24-19     18-14    13        12-5          4-0
    +----+-------+--------+------------------------------------------+
    | 10 | 00000 | 100001 |   rs1   | 1/0 |      simm12/rs2          |
    +----+-------+--------+------------------------------------------+

*/
static int disassemble(uint32 instr, void *state)
{
	int op3 = 0, op;

	op = bits(instr, OP_OFF_last, OP_OFF_first);
	op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

	if( (instr & TSUBCC_CODE_MASK) && (op == OP) && (op3 == OP3) )
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
