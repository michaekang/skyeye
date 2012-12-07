#ifndef __SKYEYE_FP_H__
#define __SKYEYE_FP_H__
#include <skyeye_types.h>
/* define SWAP for 386/960 reverse-byte-order brain-damaged CPUs */
typedef long double	float128_t;

typedef union double_long {
	double value;
	uint64_t data;
}d_precision;

typedef union float_long
{
	float value;
	uint32_t data;
}s_precision;

#endif
