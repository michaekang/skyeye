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
* @file lt_top.cpp
* @brief the binding of target and initiator
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-08
*/

#include "lt_top.h"
#if 1 
Lt_top::Lt_top(sc_core::sc_module_name module_name)
:sc_module    
(module_name
)
,m_bus("bus")
,core_initiator("core_initiator")
,mem_target("mem_target")
,uart_target("uart_target")
,arm_initiator("arm_initiator")
{
	//printf("In %s\n", __FUNCTION__);
	//arm_initiator.trans_ptr->initiator_socket.bind(mem_target.memop_socket);
	core_initiator.initiator_socket(m_bus.target_bus[0].target_socket);
	//mem_initiator.initiator_socket(mem_target.target_socket);
	/* add the memory device */
	m_bus.addDevice(0x00000000, 0xfffffff0);
	m_bus.bus_initiator[0].initiator_socket(mem_target.target_socket);

	/* add uart device */
	//uart_initiator.initiator_socket(uart_target.target_socket);
	m_bus.addDevice(0x00000000, 0xfffffff0);
	m_bus.bus_initiator[1].initiator_socket(uart_target.target_socket);
};
#endif
//Lt_top::Lt_top(){};
