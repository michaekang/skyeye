/*
        bus_recoder.c - record the activities of bus
        Copyright (C) 2003-2007 Skyeye Develop Group
        for help please send mail to <skyeye-developer@lists.sf.linuxforum.net>

        This program is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
/*
 * 08/07/2013   Kewei.Yu <keweihk@gmail.com>
 */


#include <skyeye_log.h>

#include <skyeye_types.h>
#include <memory_space.h>
#include <skyeye_addr_space.h>
#include <skyeye_mm.h>
#include "skyeye_obj.h"
#include "skyeye_class.h"
#include "skyeye_device.h"
#include "skyeye_interface.h"

uint32_t dev_get_regvalue_by_id(char* device_name, uint32_t id)
{
	conf_object_t* dev_conf = get_conf_obj(device_name);
	if(dev_conf == NULL){
		printf("Can't find the device %s!\n", device_name);
		exit(1);
	}
	skyeye_reg_intf* reg_intf = (skyeye_reg_intf*)SKY_get_interface(dev_conf, SKYEYE_REG_INTF);
	if(reg_intf == NULL){
		printf("%s does't have register interface!\n", device_name);
		exit(1);
	}
	uint32_t data = reg_intf->get_regvalue_by_id(dev_conf, id);
	return data;
}

char* dev_get_regname_by_id(char* device_name, uint32_t id)
{
	conf_object_t* dev_conf = get_conf_obj(device_name);
	if(dev_conf == NULL){
		printf("Can't find the device %s!\n", device_name);
		exit(1);
	}
	skyeye_reg_intf* reg_intf = (skyeye_reg_intf*)SKY_get_interface(dev_conf, SKYEYE_REG_INTF);
	if(reg_intf == NULL){
		printf("%s does't have register interface!\n", device_name);
		exit(1);
	}
	return reg_intf->get_regname_by_id(dev_conf, id);
}
