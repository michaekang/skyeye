#ifndef __LT_UART_TARGET_H__
#define __LT_UART_TARGET_H__

#include "skyeye_types.h"
#include "bank_defs.h"
#include "tlm.h"
#include "systemc.h"
#include "tlm_utils/simple_target_socket.h"

class Lt_uart_target:public sc_core::sc_module{
public:
	//Lt_uart_target();
	Lt_uart_target(sc_core::sc_module_name module_name);
	~Lt_uart_target();
#if 1
	/*blocking transport*/
	void custom_b_transport(tlm::tlm_generic_payload &payload
	             ,sc_core::sc_time &delay_time);

	/*bus_read/write*/
	int bus_read(short size, generic_address_t addr, uint32_t * value);
	int bus_write(short size, generic_address_t addr, uint32_t value);
	
	typedef tlm::tlm_generic_payload *gp_ptr;
	tlm_utils::simple_target_socket<Lt_uart_target> target_socket;
#endif	
};

#endif
