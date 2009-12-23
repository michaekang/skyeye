/*
 * =====================================================================================
 *
 *       Filename:  i_wry.c
 *
 *    Description:  Implementst the WRPSR SPARC instruction
 *
 *        Version:  1.0
 *        Created:  16/04/08 18:09:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

/*  Format (3)

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| op |   rd  |  op3   |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

op              = 10
rd              = 0000
op3             = 110001

31-30  29-25   24-19     18-14    13        12-5          4-0
+----+-------+--------+------------------------------------------+
| 10 | 00000 | 110001 |   rs1   | i=0 |   non-used   |    rs2    |
+----+-------+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/debug.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int rs1, rs2, imm;

#define WRPSR_CYCLES    1
#define WRPSR_CODE_MASK 0x81880000
#define PRIVILEDGE  1

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x2

#define OP3_OFF_first     19
#define OP3_OFF_last      24
#define OP3         0x31

#define RD_OFF_first      25
#define RD_OFF_last       29

#define RS1_OFF_first     14
#define RS1_OFF_last      18

#define RS2_OFF_first     0
#define RS2_OFF_last      4

#define I_OFF       13

#define SIMM13_OFF_first  0
#define SIMM13_OFF_last   12


sparc_instruction_t i_wrpsr = {
    execute,
    disassemble,
    WRPSR_CODE_MASK,
    PRIVILEDGE,
    OP,
};



static int execute(void *state)
{
    uint32 value;

    if( bit(PSRREG, PSR_S) != PRIVILEDGE )
    {
        traps->signal(PRIVILEGED_INSTR);
        return(WRPSR_CYCLES);
    }


    if( imm < 0 )
    {
        value = REG(rs1) ^ REG(rs2);
        DBG("wr reg[%d], reg[%d], psr\n", rs1, rs2);
    }
    else
    {
        value = REG(rs1) ^ sign_ext13(imm);
        DBG("wr reg[%d], 0x%x, psr\n", rs1, sign_ext13(imm));
    }


    uint32 ncwp = bits(value, PSR_CWP_last, PSR_CWP_first);
    if(ncwp >= N_WINDOWS)
    {
        traps->signal(ILLEGAL_INSTR);
        return 0;
    }

    /*  Clear the reserved bits and the ver, impl bits  */
    clear_bits(value, PSR_reserved_last, PSR_reserved_first);
    clear_bits(value, PSR_impl_last, PSR_ver_first);

    PSRREG = value;
    iu_set_cwp(ncwp);

    PCREG = NPCREG;
    NPCREG += 4;

    // Everyting takes some time
    return WRPSR_CYCLES;
}

static int disassemble(uint32 instr, void *state)
{
    int op3 = 0, op;

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    op3 = bits(instr, OP3_OFF_last, OP3_OFF_first);

    if( (instr & WRPSR_CODE_MASK) && (op == OP) && (op3 == OP3) )
    {
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

