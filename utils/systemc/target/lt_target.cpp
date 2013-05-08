/* Copyright (C) 
* 2013 - Michael.Kang blackfin.kang@gmail.com
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/
/**
* @file lt_target.cpp
* @brief The transport implementation of target
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-07
*/

#include "lt_target.h"
#include <skyeye_bus.h>

Lt_target::Lt_target(sc_core:: sc_module_name module_name)
:sc_module   (module_name)
{
	/*register the custom_b_transport as the real blocking transport function*/
	target_socket.register_b_transport(this, &Lt_target::custom_b_transport);
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
	if(bus_side){
		bus_dispatch();
	}
	else{ /* device side */
		if(tlm::TLM_READ_COMMAND == command){
			//sc_bus_read(size, address, (uint32_t*)data);
			this->device_read();
			//printf("read command, size=0x%x, addr=0x%x, data=0x%x\n", size, address, data);
		}
		if(tlm::TLM_WRITE_COMMAND == command){
			//sc_bus_write(size, address, (unsigned long)data);
			this->device_write();
			//printf("write command, size=0x%x, addr=0x%x, data=0x%x\n", size, address, data);
		}
	}
	payload.set_response_status(tlm::TLM_OK_RESPONSE);
}

int Lt_target::sc_bus_read(short size, generic_address_t addr, uint32_t * value)
{
	int res = default_bus_read( size,  addr,   value);
	return res;
}

int Lt_target::sc_bus_write(short size, generic_address_t addr, uint32_t value)
{
	int res = default_bus_write( size,  addr,  value);
	return res;
}
#endif
