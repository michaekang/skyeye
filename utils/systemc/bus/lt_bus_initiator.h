
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
* @file lt_bus_initiator.h
* @brief 
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-08
*/

#ifndef __LT_BUS_INITIATOR_H__
#define __LT_BUS_INITIATOR_H__

#include "bank_defs.h"
//#include "armdefs.h"
//#include "armmmu.h"
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"

class Lt_bus_initiator:public sc_core::sc_module{
public:
	Lt_bus_initiator(sc_core::sc_module_name module_name);
	~Lt_bus_initiator();

	void initiator_thread(tlm::tlm_generic_payload *transaction_ptr);

	int bus_read (short size, generic_address_t addr, uint32_t * value);

	int bus_write(short size, generic_address_t addr, uint32_t value);
	
	tlm::tlm_generic_payload *gp_ptr;     //generic payload
	tlm_utils::simple_initiator_socket<Lt_bus_initiator> initiator_socket;
	
};

#endif
