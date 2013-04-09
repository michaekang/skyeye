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
* @file image.h
* @brief image for ram, flash or other memory device
* @author David Yu keweihk@gmail.com
* @version 78.77
* @date 2013-04-08
*/

#ifndef __IMAGE_H
#define __IMAGE_H__

typedef struct image_ptr{
	uint32_t start_addr;
	uint32_t end_addr;
	uint32_t* mem_ptr;
}image_ptr_t;

typedef struct image_module{
	conf_object_t* obj;
	image_ptr_t* image_ptr;		//256
	uint32_t size;
	uint8_t init_value;
	char* file;
}image_module_t;

int get_parameter(char* result, char* args, const char* param);

#endif
