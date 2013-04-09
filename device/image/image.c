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
* @file image.c
* @brief The implementation of system controller
* @author David Yu keweihk@gmail.com
* @version 78.77
* @date 2013-04-08
*/

#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h> 
#include <memory_space.h>
#include <skyeye_device.h>
#define DEBUG
#include <skyeye_log.h>

#include "image.h"

char* image_attr[] = {"size", "init_value", "files"};
static exception_t image_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	image_module_t *dev = opaque->obj;
	int index;
	//parse the parameter
	for(index = 0; index < 3; index++){
		if(!strncmp(attr_name, image_attr[index], strlen(image_attr[index])))
			break;
	}
	switch(index){
		case 0:
			dev->size = value->u.integer;
			printf("size 0x%x\n", value->u.integer);
			break;
		case 1:
			break;
		case 2:
			break;
		default:
			printf("parameter error\n");
			return Invarg_exp;
	}
	return No_exp;
}

static exception_t image_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	image_module_t *dev = opaque->obj;
	return No_exp;
}

static exception_t image_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	image_module_t *dev = opaque->obj;
	return No_exp;
}

static conf_object_t* new_image(char* obj_name)
{
	image_module_t* dev= skyeye_mm_zero(sizeof(image_module_t));
	image_ptr_t* image_ptr = skyeye_mm_zero(sizeof(image_ptr_t) * 256);
	dev->image_ptr = image_ptr;
	dev->obj = new_conf_object(obj_name, dev);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->read = image_read;
	io_memory->write = image_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	
	return dev->obj;
}

static void free_image(conf_object_t* dev){
	
}

static exception_t reset_image(conf_object_t* opaque, const char* args)
{
	image_module_t *dev = opaque->obj;
	char result[64];
	int index;
	attr_value_t value;
	if(args == NULL){
		//default operations
		return No_exp;
	}else{
		//parse the parameter
		for(index = 0; index < 3; index++){
			if(!strncmp(args, image_attr[index], strlen(image_attr[index])))
				break;
		}
	}
	int ret = 0;
	switch(index){
		case 0:
			ret = get_parameter(result, args, "size");
			value.u.integer = strtoul(result, NULL, 0);
			printf("size %s\n", result);
			break;
		case 1:
			ret = get_parameter(result, args, "init_value");
			printf("init %s\n", result);
			break;
		case 2:
			ret = get_parameter(result, args, "files");
			printf("files %s\n", result);
			break;
		default:
			printf("parameter error\n");
			return Invarg_exp;
	}
	return No_exp;
}

void init_image(){
	static skyeye_class_t class_data = {
		.class_name = "image",
		.class_desc = "image module",
		.new_instance = new_image,
		.free_instance = free_image,
		.reset_instance = reset_image,
		.get_attr = NULL,
		.set_attr = image_set_attr 
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}
