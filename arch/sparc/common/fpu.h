/*
 *=====================================================================================
 *
 *	Filename:  fpu.h
 *
 *	Description:  SPARC FPU Unit
 *
 *	Revision:  none
 *	Compiler:  gcc
 *
 *	Author:  David Yu, keweihk@gmail.com
 *	Company: Tsinghua, skyeye team
 *
 *=====================================================================================
 */ 
#ifndef _FPU_H_
#define _FPU_H_

#ifndef __SPARC_H__
#error "arch/sparc/common/sparc.h header file must be included"
#endif
#ifndef _TRAPS_H_
#error "arch/sparc/common/traps.h header file must be included"
#endif

#include <stdio.h>
#include <skyeye_fp.h>
#include "iu.h"


#define FPU_NUMBER              (1<<0)
#define FPU_ZERO                (1<<1)
#define FPU_DENORMAL            (1<<2)
#define FPU_INFINITY            (1<<3)
#define FPU_NAN                 (1<<4)
#define FPU_NAN_SIGNAL          (1<<5)

#define FPU_QNAN                (FPU_NAN)
#define FPU_SNAN                (FPU_NAN|FPU_NAN_SIGNAL)
/* IEEE_754 floating-point exceptions */
#define IEEE_FNV		(1<<4)	
#define IEEE_FOF		(1<<3)
#define IEEE_FUF		(1<<2)
#define IEEE_FDZ		(1<<1)
#define IEEE_FDX		(1<<0)


/*
 * Double-precision
 */
typedef struct fpu_double {
	int16_t		exponent;
	uint16_t	sign;
	uint64_t	significand;
}fp_double_t;

/* define for estimate double float over or under flow */
d_precision     D_max, D_min;
#define FPU_DOUBLE_MANTISSA_BITS        (52)
#define FPU_DOUBLE_EXPONENT_BITS        (11)
#define FPU_DOUBLE_LOW_BITS             (64 - FPU_DOUBLE_MANTISSA_BITS - 2)
#define FPU_DOUBLE_LOW_BITS_MASK        ((1 << FPU_DOUBLE_LOW_BITS) - 1)

/*
 * The bit in an unpacked double which indicates that it is a quiet NaN
 */
#define FPU_DOUBLE_SIGNIFICAND_QNAN     (1ULL << (FPU_DOUBLE_MANTISSA_BITS - 1 + FPU_DOUBLE_LOW_BITS))

/*
 * Operations on packed single-precision numbers
 */
#define fpu_double_packed_sign(v)       ((v) & (1ULL << 63))
#define fpu_double_packed_negate(v)     ((v) ^ (1ULL << 63))
#define fpu_double_packed_abs(v)        ((v) & ~(1ULL << 63))
#define fpu_double_packed_exponent(v)   (((v) >> FPU_DOUBLE_MANTISSA_BITS) & ((1 << FPU_DOUBLE_EXPONENT_BITS) - 1))
#define fpu_double_packed_mantissa(v)   ((v) & ((1ULL << FPU_DOUBLE_MANTISSA_BITS) - 1))
static inline uint32_t fls(int x)
{
	int r = 32;

	if (!x)  
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}       
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}       

static void fpu_double_dump(const char *str, fp_double_t *d)
{
	printf("VFP: %s: sign=%d exponent=%d significand=%016llx\n",
			str, d->sign != 0, d->exponent, d->significand);
}

/*
 * Unpack a double-precision float.  Note that this returns the magnitude
 * of the double-precision float mantissa with the 1. if necessary,
 * aligned to bit 62.
 */
static inline void fpu_double_unpack(fp_double_t *s, int64_t val)
{
	uint64_t significand;

	s->sign = fpu_double_packed_sign(val) >> 48;
	s->exponent = fpu_double_packed_exponent(val);

	significand = (uint64_t) val;
	significand = (significand << (64 - FPU_DOUBLE_MANTISSA_BITS)) >> 2;
	if (s->exponent && s->exponent != 2047)
		significand |= (1ULL << 62);
	s->significand = significand;
}

/*
 * Re-pack a double-precision float.  This assumes that the float is
 * already normalised such that the MSB is bit 30, _not_ bit 31.
 */
static inline int64_t fpu_double_pack(fp_double_t *s)
{
	uint64_t val;
	val = ((uint64_t)s->sign << 48) +
		((uint64_t)s->exponent << FPU_DOUBLE_MANTISSA_BITS) +
		(s->significand >> FPU_DOUBLE_LOW_BITS);
	return (int64_t)val;
}

static inline int fpu_double_type(fp_double_t *s)
{
	int type = FPU_NUMBER;
	fpu_double_dump("fpu_double_type", s);
	if (s->exponent == 2047) {
		if (s->significand == 0)
			type = FPU_INFINITY;
		else if (s->significand & FPU_DOUBLE_SIGNIFICAND_QNAN)
			type = FPU_QNAN;
		else
			type = FPU_SNAN;
	} else if (s->exponent == 0) {
		if (s->significand == 0)
			type |= FPU_ZERO;
		else
			type |= FPU_DENORMAL;
	}
	return type;
}

static void fpu_double_normalise_denormal(fp_double_t *fd)
{               
	int bits = 31 - fls(fd->significand >> 32);
	if (bits == 31)
		bits = 63 - fls(fd->significand);

	if (bits) {
		fd->exponent -= bits - 1;
		fd->significand <<= bits;
	}
}
extern uint64_t fpu_get_double(unsigned int reg);
extern void fpu_put_double(uint64_t val, unsigned int reg);
extern int ieee_754_exception(uint32_t cexc);

enum{
	FSR_cexc_first = 0,
	FSR_cexc_last = 4,
	FSR_aexc_first = 5,
	FSR_aexc_last = 9,
	FSR_fcc_first = 10,
	FSR_fcc_last = 11,
	FSR_qne = 13,
	FSR_ftt_first = 14,
	FSR_ftt_last = 16,
	FSR_ver_first = 17,
	FSR_ver_last = 19,
	FSR_res_first = 20,
	FSR_res_last = 21,
	FSR_NS = 22,
	FSR_TEM_first = 23,
	FSR_TEM_last = 27,
	FSR_RD_first = 30,
	FSR_RD_last = 31,
};

enum{
	None = 0,
	IEEE_754_exception,
	unfinished_FPop,
	unimplemented_FPop,
	sequence_error,
	hardwaree_error,
	invalid_fp_register,
	reserved,
};

/* ftt must be 0 before fp exception occur*/
#define fsr_set_ftt(x)	FPSRREG |= (x << FSR_ftt_first)
#define fsr_clear_ftt()	FPSRREG &= ~(7 << FSR_ftt_first)
#define fsr_clear_rd()	FPSRREG &= ~(3 << FSR_RD_first)
#define fsr_set_rd(x)	fsr_clear_rd();					\
					FPSRREG |= (x << FSR_RD_first)
#define fsr_set_cexc(x)	FPSRREG &= ~(0x1f << FSR_cexc_first);		\
				   FPSRREG |= x << FSR_cexc_first
#define fsr_get_cexc() (FPSRREG >> FSR_cexc_first) & 0x1f

#define fsr_set_aexc(x)	FPSRREG &= ~(0x1f << FSR_aexc_first);		\
				   FPSRREG |= (x << FSR_aexc_first)
#define fsr_get_TEM()	((FPSRREG >> FSR_TEM_first) & 0x1f)

#define fsr_set_fcc(x)	FPSRREG &= ~(0x3 << FSR_fcc_first);		\
				   FPSRREG |= (x << FSR_fcc_first)

#endif
