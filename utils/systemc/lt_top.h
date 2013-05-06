#ifndef __LT_TOP_H__
#define __LT_TOP_H__

#include "sc_skyeye_arm.h"
#include "lt_target.h"

class Lt_top
:public sc_core::sc_module
{
public:
	//Lt_top();
	Lt_top(sc_core::sc_module_name module_name);
	~Lt_top(){};

	Lt_target mem_target;
	sc_skyeye_arm arm_initiator;
};
#endif
