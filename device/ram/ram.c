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
* @file ram.c
* @brief The implementation of ram
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
#include <skyeye_arch.h>
#include <skyeye_swapendian.h>
#define DEBUG
#include <skyeye_log.h>

#include "ram.h"

static char* ram_attr[] = {"image"};
static exception_t ram_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	ram_module_t *dev = opaque->obj;
	int index;
	//parse the parameter
	for(index = 0; index < 3; index++){
		if(!strncmp(attr_name, ram_attr[index], strlen(ram_attr[index])))
			break;
	}
	switch(index){
		case 0:
			dev->image = (memory_space_intf*)SKY_get_interface(value->u.object, MEMORY_SPACE_INTF_NAME);
			break;
		default:
			printf("parameter error\n");
			return Invarg_exp;
	}
	return No_exp;
}

static exception_t ram_read_word(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint32_t data;
	ram_device_t* dev = opaque->obj;
	exception_t ret = dev->image->read(dev->image->conf_obj, offset, &data, 4);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		*(uint32_t*)buf = data;
	else if(arch_instance->endianess == Big_endian)
		*(uint32_t*)buf = word_from_BE(data);

	return No_exp;
}

static exception_t ram_read_halfword(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint16_t data;
	ram_device_t* dev = opaque->obj;
	exception_t ret = dev->image->read(dev->image->conf_obj, offset, &data, 2);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		*(uint16_t*)buf = data;
	else if(arch_instance->endianess == Big_endian)
		*(uint16_t*)buf = half_from_BE(data);
	return No_exp;
}

static exception_t ram_read_byte(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint8_t data;
	ram_device_t* dev = opaque->obj;
	exception_t ret = dev->image->read(dev->image->conf_obj, offset, &data, 1);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		*(uint8_t*)buf = data;
	else if(arch_instance->endianess == Big_endian)
		*(uint8_t*)buf = data;

	return No_exp;
}

static exception_t ram_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	ram_module_t *dev = opaque->obj;
	switch(count){
		case 4:
			return  ram_read_word(opaque, offset,  buf);
		case 2:
			return  ram_read_halfword(opaque, offset,  buf);
		case 1:
			return  ram_read_byte(opaque, offset,  buf);
		default:
			printf("In %s, count %d is error!\n", __func__, count);
			break;
	}

	return No_exp;
}

static exception_t ram_write_word(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint32_t data;
	ram_device_t* dev = opaque->obj;
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		data = *(uint32_t*)buf;
	else if(arch_instance->endianess == Big_endian)
		data = word_from_BE(*(uint32_t*)buf);
	exception_t ret = dev->image->write(dev->image->conf_obj, offset, &data, 4);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}

	return No_exp;
}

static exception_t ram_write_halfword(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint16_t data;
	ram_device_t* dev = opaque->obj;
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		data = *(uint16_t*)buf;
	else if(arch_instance->endianess == Big_endian)
		data = half_from_BE(*(uint16_t*)buf);
	exception_t ret = dev->image->write(dev->image->conf_obj, offset, &data, 2);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}

	return No_exp;
}

static exception_t ram_write_byte(conf_object_t *opaque, generic_address_t offset, void* buf)
{
	uint8_t data;
	ram_device_t* dev = opaque->obj;
	generic_arch_t* arch_instance = get_arch_instance(NULL);
	/* judge the endianess */
	if(arch_instance->endianess == Little_endian)
		data = *(uint8_t*)buf;
	else if(arch_instance->endianess == Big_endian)
		data = *(uint8_t*)buf;
	exception_t ret = dev->image->write(dev->image->conf_obj, offset, &data, 1);
	if(ret != No_exp){
		printf("In %s, %s Not_found offset 0x%x\n", __func__, dev->obj->class_name, offset);
		return Not_found_exp;
	}

	return No_exp;
}

static exception_t ram_write(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	switch(count){
		case 4:
			return  ram_write_word(opaque, offset,  buf);
		case 2:
			return  ram_write_halfword(opaque, offset,  buf);
		case 1:
			return  ram_write_byte(opaque, offset,  buf);
		default:
			printf("In %s, count %d is error!\n", __func__, count);
	}
	return No_exp;
}

static conf_object_t* new_ram(char* obj_name)
{
	ram_module_t* dev= skyeye_mm_zero(sizeof(ram_module_t));
	dev->obj = new_conf_object(obj_name, dev);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->read = ram_read;
	io_memory->write = ram_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	

	return dev->obj;
}

static void free_ram(conf_object_t* dev){
	
}

static exception_t reset_ram(conf_object_t* opaque, const char* args)
{
	ram_module_t *dev = opaque->obj;
	char result[64];
	int index;
	attr_value_t value;
	if(args == NULL){
		//default operations
		return No_exp;
	}else{
		//parse the parameter
		for(index = 0; index < 3; index++){
			if(!strncmp(args, ram_attr[index], strlen(image_attr[index])))
				break;
		}
	}
	return No_exp;
}

void init_ram(){
	static skyeye_class_t class_data = {
		.class_name = "ram",
		.class_desc = "ram module",
		.new_instance = new_ram,
		.free_instance = free_ram,
		.reset_instance = reset_ram,
		.get_attr = NULL,
		.set_attr = ram_set_attr 
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}
