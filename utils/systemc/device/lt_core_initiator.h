#ifndef __LT_CORE_INITIATOR_H__
#define __LT_CORE_INITIATOR_H__

#include "bank_defs.h"
//#include "armdefs.h"
//#include "armmmu.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

class Lt_core_initiator:public sc_core::sc_module{
public:
	Lt_core_initiator(sc_core::sc_module_name module_name);
	~Lt_core_initiator();

	void initiator_thread(tlm::tlm_generic_payload *transaction_ptr);

	int bus_read (short size, generic_address_t addr, uint32_t * value);

	int bus_write(short size, generic_address_t addr, uint32_t value);
	
	tlm::tlm_generic_payload *gp_ptr;     //generic payload
	tlm_utils::simple_initiator_socket<Lt_core_initiator> initiator_socket;
	
};

#endif
