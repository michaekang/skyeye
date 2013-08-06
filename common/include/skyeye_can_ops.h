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
* @file skyeye_can_ops.h
* @brief The interface for can operation
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-06
*/
#ifndef __SKYEYE_CAN_OPS_H__
#define __SKYEYE_CAN_OPS_H__

typedef struct can_ops_intf{
	conf_object_t* obj;
	exception_t (*start)(conf_object_t* obj);	
	exception_t (*stop)(conf_object_t* obj);	
	exception_t (*transmit)(conf_object_t* obj, void* buf, int nbytes);
	exception_t (*receive)(conf_object_t* obj, void* buf, int nbytes);	
}can_ops_intf;
#define CAN_OPS_INTF_NAME "can_ops_intf"

#endif
