/* Copyright (C) 
* 2013 - David Yu keweihk@gmail.com
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
* @file ram.h
* @brief ram for flash or other memory device
* @author David Yu keweihk@gmail.com
* @version 78.77
* @date 2013-04-08
*/

#ifndef __RAM_H__
#define __RAM_H__

typedef struct ram_device{
	conf_object_t* obj;
	memory_space_intf* image;
}ram_device_t;

#endif
