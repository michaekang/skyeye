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
* @file sc_init.cpp
* @brief the systemc initialization
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-07
*/

#include "lt_top.h"
#include "sc_init.h"
#include <skyeye_bus.h>

Lt_top *top_ptr;
static int sc_core_read(short size, generic_address_t addr, uint32_t * value){
	int ret;
	//ret = default_bus_read(size, addr, value);
	//printf("In %s, size=0x%x, addr=0x%x, value=0x%x\n", __FUNCTION__, size, addr, value);
	ret = top_ptr->core_initiator.bus_read(size, addr, value);
	return ret;
}
static int sc_core_write(short size, generic_address_t addr, uint32_t value){
	int ret;
	//ret = default_bus_write(size, addr, value);
	//printf("In %s, size=0x%x, addr=0x%x, value=0x%x\n", __FUNCTION__, size, addr, value);
	ret = top_ptr->core_initiator.bus_write(size, addr, value);
	return ret;
}

/**
* @brief select different initiator to dispatch access
*
* @param payload
* @param delay_time
*/
void bus_dispatch(tlm:: tlm_generic_payload & payload, sc_core :: sc_time & delay_time){
	generic_address_t   address = payload.get_address();
	tlm::tlm_command  command = payload.get_command();
	unsigned char               *data        = payload.get_data_ptr();
	short                     size         = payload.get_data_length();
	
	if((address & 0xfffff000) == 0xfffd0000){
		/* transport from uart_initiator to uart_target*/
		if((tlm::TLM_READ_COMMAND) == command){
			top_ptr->uart_initiator.bus_read(size, address, (uint32_t *)data);
		}
		else if(tlm::TLM_WRITE_COMMAND == command){
			top_ptr->uart_initiator.bus_write(size, address, (unsigned long)data);
		}
		else{}
	}
	else if(address >= 0x01000000 && address < (0x01000000 + 0x00400000)){
		if((tlm::TLM_READ_COMMAND) == command){
			top_ptr->mem_initiator.bus_read(size, address, (uint32_t *)data);
		}
		else if(tlm::TLM_WRITE_COMMAND == command){
			top_ptr->mem_initiator.bus_write(size, address, (unsigned long)data);
		}
	}
	else{
		//something wrong
	}
	return;
}
void init_systemc_class(){
	register_bus_operation(sc_core_read, sc_core_write);
	top_ptr = new Lt_top("lt_top");
	//new uart_device()
	//map_uart_device();
	return;
}
