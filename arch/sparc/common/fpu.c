/*
 * =====================================================================================
 * 	Description:  fpu.c
 *
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  David Yu, keweihk@gmail.com
 *        Company:  Tsinghua skyeye team
 * =====================================================================================
 */
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "sparc.h"
#include "traps.h"
#include "bits.h"
#include "iu.h"
#include "debug.h"
#include "stat.h"
#include "fpu.h"

#include "skyeye_config.h"

extern sparc_return_t iu_i_register(sparc_instruction_t *i_new);
extern sparc_instruction_t i_fdivd;
extern sparc_instruction_t i_fstod;
extern sparc_instruction_t i_fcmpd;
extern sparc_instruction_t i_fcmps;
extern sparc_instruction_t i_fbne;
extern sparc_instruction_t i_fbuge;
extern sparc_instruction_t i_fbule;
extern sparc_instruction_t i_fbo;
extern sparc_instruction_t i_fbe;

static void fpu_init_state(void)
{
	/* define for estimate double float over or under flow */
	D_max.data = 0x7fefffffffffffffUL;
	D_min.data = 0x0010000000000000UL;
	FPSRREG = 0x0;
}
static void fpu_isa_register(void)
{
	iu_i_register(&i_fdivd);
	iu_i_register(&i_fstod);
	iu_i_register(&i_fcmpd);
	iu_i_register(&i_fbne);
	iu_i_register(&i_fbe);
	iu_i_register(&i_fbuge);
	iu_i_register(&i_fbule);
	iu_i_register(&i_fbo);
	iu_i_register(&i_fcmps);
}

int init_sparc_fpu(void)
{
	fpu_init_state();
	fpu_isa_register();
	return 0;
}

uint32_t fpu_get_single(unsigned int reg)
{
	uint32_t val = 0;
	val = FPREG(reg);
	return val;
}

void fpu_put_single(uint64_t val, unsigned int reg)
{
	FPREG(reg) = val;
}

uint64_t fpu_get_double(unsigned int reg)
{
	uint64_t val = 0;
	val = ((uint64_t)FPREG(reg) << 32 | FPREG(reg + 1));
	return val;
}

void fpu_put_double(uint64_t val, unsigned int reg)
{
	FPREG(reg) = (uint32_t)(val >> 32);
	FPREG(reg + 1) = val;
}

void fpu_setfsr_exc(uint32_t x)
{
	uint32_t tmp;
	tmp = fsr_get_cexc();
	fsr_set_aexc(tmp);
	fsr_set_cexc(x);
}

/* 
 * exception occur return 1, otherwise 0.
 */
int ieee_754_exception(uint32_t cexc)
{
	uint32_t tem;
	fpu_setfsr_exc(cexc);
	if((cexc & tem) != 0){
		/* ieee 754 exception occur*/
		traps->signal(FP_EXCEPTION);
		fsr_set_ftt(IEEE_754_exception);
		return 1;
	}

	return 0;
}

uint32_t get_double_type(double x)
{
	uint32_t type = 0;
	if((isinf(x)) != 0)
		type |= FPU_INFINITY;
	if((isnan(x)) != 0)
		type |= FPU_NAN;
	if(x == 0)
		type |= FPU_ZERO;

	return type;
}
