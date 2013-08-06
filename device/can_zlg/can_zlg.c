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
* @file can_ops.c
* @brief The interface for zlg USB CAN I device
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-06
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

#include "skyeye_can_ops.h"
//#include "ControlCan.h"
#include "can_zlg.h"

exception_t open_can_device(){
	int nDeviceType = 20; /* USBCAN-E-U */
	int nDeviceInd = 0;
	int nReserved = 9600;
	exception_t ret;
	#if 0
	DWORD dwRel;
	dwRel = VCI_openDevice(nDeviceType, nDeviceInd, nReserved);
	if(dwRel != STATUS_OK){
		return
	}
	dwRel = VCI_InitCAN(nDeviceType, nDeviceInd, nCANInd, &vic);
	if(dwRel == STATUS_ERR){
	}
	#endif
	return ret;
}
exception_t close_can(){
	exception_t ret;
	//VCI_CloseDevice(nDeviceType, nDev)
	return ret;
}
exception_t stop_can(){
	exception_t ret;
	return ret;
}

exception_t start_can(){
	exception_t ret;
	return ret;
}
exception_t can_transmit(conf_object_t* obj, void* addr, int nbytes){
	can_zlg_device *dev = obj->obj;
	exception_t ret;
	#if 0
	VCI_CAN_OBJ vco;
	memset(&vco, '\0', sizeof(VCI_CAN_OBJ));
	vco.ID = 0x00000000;
	vco.SendType = 0;
	vco.RemoteFlag = 0;
	vco.ExternFlag = 0;
	vco.DataLen = 8;
	lReg = VCI_Transmit(nDeviceType, nDeviceInd, nCANInd, &vco, i);
	#endif
	return ret;
}

exception_t can_receive(conf_object_t* obj, void* addr, int nbytes){
	exception_t ret;
	#if 0
	VCI_CAN_OBJ vco[100];
	lRet = VCI_Receive(nDeviceType, nDeviceInd, nCANInd, vco, 100, 400);
	#endif
	return ret;
}
static conf_object_t* new_can_zlg(char* obj_name){
	can_zlg_device* dev = skyeye_mm_zero(sizeof(can_zlg_device));
	dev_info_t* info =  skyeye_mm_zero(sizeof(dev_info_t));
	dev->obj = new_conf_object(obj_name, dev);
	dev->info = info;

	can_ops_intf* ops = skyeye_mm_zero(sizeof(can_ops_intf));
	ops->start = start_can;
	ops->stop = stop_can;
	ops->transmit = can_transmit;
	ops->receive = can_receive;
	dev->ops = ops;
	SKY_register_interface(ops, obj_name, CAN_OPS_INTF_NAME);	

	info->device_type = 20;
	info->device_id = 0;
	info->reserved = 9600; /* baud rate */
	return dev->obj;
}
void free_can_zlg(conf_object_t* dev){
	
}

void init_can_zlg(){
	static skyeye_class_t class_data = {
		.class_name = "can_zlg",
		.class_desc = "CAN USB I device of ZLG",
		.new_instance = new_can_zlg,
		.free_instance = free_can_zlg,
		.get_attr = NULL,
		.set_attr = NULL
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}
