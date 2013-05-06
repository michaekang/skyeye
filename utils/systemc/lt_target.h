#ifndef __LT_TARGET_H__
#define __LT_TARGET_H__

#include "skyeye_types.h"
#include "bank_defs.h"
#include "tlm.h"
#include "systemc.h"
#include "tlm_utils/simple_target_socket.h"

class Lt_target:public sc_core::sc_module{
public:
	//Lt_target();
	Lt_target(sc_core::sc_module_name module_name);
	~Lt_target();
#if 1
	/*blocking transport*/
	void custom_b_transport(tlm::tlm_generic_payload &payload
	             ,sc_core::sc_time &delay_time);

	/*bus_read/write*/
	int sc_bus_read(short size, generic_address_t addr, uint32_t * value);
	int sc_bus_write(short size, generic_address_t addr, uint32_t value);
	
	typedef tlm::tlm_generic_payload *gp_ptr;
	tlm_utils::simple_target_socket<Lt_target> memop_socket;
#endif	
};

#endif
