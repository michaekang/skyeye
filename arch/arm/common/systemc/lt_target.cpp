#include "lt_target.h"

Lt_target::Lt_target(sc_core:: sc_module_name module_name)
:sc_module   (module_name)
{
	/*register the custom_b_transport as the real blocking transport function*/
	memop_socket.register_b_transport(this, &Lt_target::custom_b_transport);
}

Lt_target::~Lt_target()
{
}
#if 1 
void Lt_target::custom_b_transport( tlm:: tlm_generic_payload & payload, sc_core :: sc_time & delay_time)
{
	generic_address_t   address = payload.get_address();
	tlm::tlm_command  command = payload.get_command();
	unsigned char               *data        = payload.get_data_ptr();
	short                     size         = payload.get_data_length();

	if(tlm::TLM_READ_COMMAND == command)
		sc_bus_read(size, address, (uint32_t*)data);
	if(tlm::TLM_WRITE_COMMAND == command)
		sc_bus_write(size, address, *(uint32_t*)data);

	payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

int Lt_target::sc_bus_read(short size, generic_address_t addr, uint32_t * value)
{
	int res = bus_read( size,  addr,   value);
	return res;
}

int Lt_target::sc_bus_write(short size, generic_address_t addr, uint32_t value)
{
	int res = bus_write( size,  addr,  value);
	return res;
}
#endif
