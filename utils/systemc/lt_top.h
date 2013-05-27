#ifndef __LT_TOP_H__
#define __LT_TOP_H__

#include <systemc.h>
#include <tlm.h>
#include "sc_skyeye_arm.h"
#include "bus/lt_bus_target.h"
#include "bus/lt_bus_initiator.h"
#include "device/lt_core_initiator.h"
#include "device/lt_mem_target.h"
#include "device/lt_uart_target.h"

class Lt_top
:public sc_core::sc_module
{
public:
	//Lt_top();
	Lt_top(sc_core::sc_module_name module_name);
	~Lt_top(){};

	SimpleBusLT<2,1>  m_bus; /* core, memory and uart */
	Lt_core_initiator core_initiator; /* core side */
	//Lt_bus_target core_target; /* bus side */

	//Lt_bus_initiator mem_initiator; /* bus side */
	Lt_mem_target mem_target; /* device side */

	//Lt_bus_initiator uart_initiator; /* bus side */
	Lt_uart_target uart_target; /* device side */

	sc_skyeye_arm arm_initiator;
};
#endif
