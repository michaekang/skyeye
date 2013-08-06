/* Copyright (C) 
* 2011 - Michael.Kang blackfin.kang@gmail.com
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
* @file can_zlg.h
* @brief The definition of HEXX for am35x
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-12-12
*/

#ifndef __AM35X_HEXX_H__
#define __AM35X_HECC_H__

typedef struct dev_info{
	uint32_t device_type;
	uint32_t device_id;
	uint32_t reserved;
}dev_info_t; 

typedef struct can_zlg_device{
	conf_object_t* obj;
	dev_info_t* info;
	can_ops_interface_t* ops;
}can_zlg_device;

#endif
