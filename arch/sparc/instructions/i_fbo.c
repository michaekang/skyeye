/*
 * =====================================================================================
 *
 *       Filename:  i_fbo.c
 *
 *    Description:  Implementation of the FBO instruction
 *
 *        Version:  1.0
 *        Created:  16/04/08 18:09:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David Yu, keweihk@gmail.com
 *        Company:  Tsinghua skyeye team
 *
 * =====================================================================================
 */

/*  Format (2)

31-30  29   28-25    24-22       21-0
+----+----+--------+------------------------------------------+
| op | a  |  cond  |   110   |          disp22                |
+----+----+--------+------------------------------------------+

op              = 00
cond            = 1110

31-30  29   28-25    24-22       21-0
+----+----+--------+------------------------------------------+
| op | a  |  1110  |   110   |          disp22                |
+----+----+--------+------------------------------------------+

*/
#include "../common/bits.h"
#include "../common/sparc.h"
#include "../common/traps.h"
#include "../common/iu.h"
#include "../common/fpu.h"
#include "i_utils.h"

#include <stdio.h>

static int execute(void *);
static int disassemble(uint32 instr, void *state);
static int cond, disp22, annul, op, fmt;

#define FBO_CYCLES    1
#define FBO_CODE_MASK 0x1f800000
#define PRIVILEDGE  0

#define OP_OFF_first     30
#define OP_OFF_last      31
#define OP         0x0

#define COND_OFF_first     25
#define COND_OFF_last      28
#define COND         0xf

#define A_OFF       29

#define DISP22_OFF_first  0
#define DISP22_OFF_last   21

#define FMT_OFF_first	22
#define FMT_OFF_last	24
#define FMT		0x6

sparc_instruction_t i_fbo = {
    execute,
    disassemble,
    FBO_CODE_MASK,
    PRIVILEDGE,
    OP,
};


static int execute(void *state)
{
    int32 addr = (disp22 << 10) >> 8;   /*  sign extend */

    int fcc = fsr_get_fcc();

    /*  Branch on not equal, not z  */
    if( fcc == 3 )
    {
        if( annul )
        {
            /*  the delay slot is annulled  */
            PCREG = NPCREG + 4;
            NPCREG += 8;
            print_inst_BICC("fbo,a", addr);
        }
        else
        {
            /*  the delay slot is executed  */
            PCREG = NPCREG;
            NPCREG += 4;
            print_inst_BICC("fbo", addr);
        }
    }
    else
    {
        addr += PCREG;
        PCREG = NPCREG;
        NPCREG = addr;
        print_inst_BICC("fbo", addr);
    }

//    DBG("fbo,a 0x%x\n", addr);

    // Everyting takes some time
    return(FBO_CYCLES);

}

static int disassemble(uint32 instr, void *state)
{

    op = bits(instr, OP_OFF_last, OP_OFF_first);
    annul = bit(instr, A_OFF);
    cond = bits(instr, COND_OFF_last, COND_OFF_first);
    fmt = bits(instr, FMT_OFF_last, FMT_OFF_first);

    if( (instr & FBO_CODE_MASK) && (op == OP) && (cond == COND) && (fmt = FMT))
    {
        disp22 = bits(instr, DISP22_OFF_last, DISP22_OFF_first);
        return 1;
    }
    return 0;
}

