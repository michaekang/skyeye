#ifndef __SC_SKYEYE_ARM_H__
#define __SC_SKYEYE_ARM_H__

#include "lt_transform.h"
#include "sim_control.h"

extern Lt_transform *transform_ptr;
SC_MODULE(sc_skyeye_arm)
{
	//Lt_transform *trans_ptr;

	SC_HAS_PROCESS(sc_skyeye_arm);

	void machine_start();

	sc_skyeye_arm(sc_core::sc_module_name module_name):
	sc_module  (module_name)
	{

		//trans_ptr = new Lt_transform("lt_transform");
		//transform_ptr = trans_ptr;

		SC_THREAD(machine_start);
	}
};
#endif
