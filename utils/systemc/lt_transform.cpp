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
* @file lt_transform.cpp
* @brief The initiator transport
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-07
*/

#include "lt_transform.h"

Lt_transform::Lt_transform(sc_core::sc_module_name module_name)
:sc_module   (module_name)
,initiator_socket("initiator_socket")
{
	gp_ptr = new tlm::tlm_generic_payload();
}
Lt_transform::~Lt_transform()
{
}


int Lt_transform::sc_a71_bus_read (short size, generic_address_t addr, uint32_t * value)
{
	gp_ptr->set_command(tlm::TLM_READ_COMMAND);
	gp_ptr->set_data_ptr((unsigned char*)value);
	gp_ptr->set_address(addr);
	gp_ptr->set_data_length(size);
	gp_ptr->set_streaming_width(size);
	gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
	initiator_thread( gp_ptr );
	return 0;
}

int Lt_transform::sc_a71_bus_write(short size, generic_address_t addr, uint32_t value)
{
	gp_ptr->set_command(tlm::TLM_WRITE_COMMAND);
	gp_ptr->set_data_ptr((unsigned char*)value);
	gp_ptr->set_address(addr);
	gp_ptr->set_data_length(size);
	gp_ptr->set_streaming_width(size);
	gp_ptr->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
	initiator_thread( gp_ptr );
	return 0;
}

void Lt_transform::initiator_thread( tlm::tlm_generic_payload * transaction_ptr)
{
	sc_time delay = SC_ZERO_TIME;
	tlm::tlm_response_status gp_status;

	initiator_socket->b_transport(*transaction_ptr, delay);           //transport the payload to target
	gp_status = transaction_ptr->get_response_status();

	if(tlm::TLM_OK_RESPONSE == gp_status)
		wait(delay);
	else
		return;
}
