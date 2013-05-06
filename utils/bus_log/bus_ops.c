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
* @file bus_ops.c
* @brief The bus operation used to log access
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-05-06
*/

#include <skyeye_types.h>
#include <skyeye_bus.h>
#include <skyeye_arch.h>
#include <skyeye_callback.h>

/**
* @brief generic data read interface
*
* @param size the width of data
* @param addr the address for the data
* @param value the value of data
*
* @return 
*/
int log_bus_read(short size, generic_address_t addr, uint32_t * value){
	default_bus_read(size, addr, value);
	bus_snoop(SIM_access_read, size ,addr, *value, After_act);
	generic_arch_t* arch_instance = get_arch_instance("");
	exec_callback(Bus_read_callback, arch_instance);
	return 0;	
}

/**
 * The interface of write data from bus
 */

/**
* @brief the generic data write interface
*
* @param size the width of data
* @param addr the address of the data
* @param value the value of the data
*
* @return 
*/
int log_bus_write(short size, generic_address_t addr, uint32_t value){
	
	bus_snoop(SIM_access_write, size ,addr, value, Before_act);
	generic_arch_t* arch_instance = get_arch_instance("");
	exec_callback(Bus_write_callback, arch_instance);
        default_bus_write(size, addr, value);
	return 0; 
}
