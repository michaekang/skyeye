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
* @file am35x_hecc.c
* @brief The implementation of system controller
* @author Michael.Kang blackfin.kang@gmail.com
* @version 78.77
* @date 2011-12-12
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
#include "am35x_hecc.h"

#define TEST_CAN 0
#if TEST_CAN
static char test_buf[8];
#endif

static exception_t am35x_hecc_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct am35x_hecc_device *dev = opaque->obj;
	hecc_reg_t* regs = dev->regs;
	switch(offset) {
		case 0x0:
			*(uint32_t*)buf = regs->dcan_ctl;
			break;		
		case 0x4:
			*(uint32_t*)buf = regs->dcan_es;
			break;
		case 0x98:
			*(uint32_t*)buf = regs->dcan_nwdat;
			break;
		case 0x100:
			*(uint32_t*)buf = regs->if1_cmd;
                        break;
		case 0x110:
			*(uint32_t*)buf = regs->if1_data_a;
                        break;
		case 0x114:
			*(uint32_t*)buf = regs->if1_data_b;
                        break;

		default:
			printf("Can not read the register at 0x%x in hecc\n", offset);
			return Invarg_exp;
	}
	//printf("In %s, offset=0x%x, data=0x%x\n", __FUNCTION__, offset, *(uint32_t*)buf);
	return No_exp;
}

static exception_t am35x_hecc_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	struct am35x_hecc_device *dev = opaque->obj;
	hecc_reg_t* regs = dev->regs;
	uint32_t val = *(uint32_t*)buf;
	switch(offset) {
		case 0x100:
			regs->if1_cmd = val;
			int msg_num = regs->if1_cmd & 0xFF;
			int rw = (regs->if1_cmd >> 23) & 0x1;
			if(msg_num > 0x1 && msg_num < 0x81){ /* valid msg number */
				/* transfer is triggered */
				regs->if1_cmd |= 0x8000; /* set BUSY bit */
				if(rw == 0){ /* read */
					#if TEST_CAN
					regs->if1_data_a = test_buf[0] | (test_buf[1] << 8) | (test_buf[2] << 16) | (test_buf[3] <<24);
					regs->if1_data_b = test_buf[4] | (test_buf[5] << 8) | (test_buf[6] << 16) | (test_buf[7] <<24);
					#else
					char msg_buf[8];
					dev->can_ops->receive(dev->can_ops->obj, msg_buf, 8);
					regs->if1_data_a = msg_buf[0] | (msg_buf[1] << 8) | (msg_buf[2] << 16) | (msg_buf[3] <<24);
					regs->if1_data_b = msg_buf[4] | (msg_buf[5] << 8) | (msg_buf[6] << 16) | (msg_buf[7] <<24);
					#endif
					regs->dcan_nwdat = 0x0;
				}
				else{ /* write */
					#if TEST_CAN
					test_buf[0] = regs->if1_data_a & 0xFF;
					test_buf[1] = (regs->if1_data_a >> 8 )& 0xFF;
					test_buf[2] = (regs->if1_data_a >> 16 ) & 0xFF;
					test_buf[3] = (regs->if1_data_a >> 24) & 0xFF;
					test_buf[4] = regs->if1_data_b & 0xFF;
					test_buf[5] = (regs->if1_data_b >> 8 )& 0xFF;
					test_buf[6] = (regs->if1_data_b >> 16 )& 0xFF;
					test_buf[7] = (regs->if1_data_b >> 24 )& 0xFF;

					#else
					char msg_buf[8];
					msg_buf[0] = regs->if1_data_a & 0xFF;
					msg_buf[1] = (regs->if1_data_a >> 8 )& 0xFF;
					msg_buf[2] = (regs->if1_data_a >> 16 ) & 0xFF;
					msg_buf[3] = (regs->if1_data_a >> 24) & 0xFF;
					msg_buf[4] = regs->if1_data_b & 0xFF;
					msg_buf[5] = (regs->if1_data_b >> 8 )& 0xFF;
					msg_buf[6] = (regs->if1_data_b >> 16 )& 0xFF;
					msg_buf[7] = (regs->if1_data_b >> 24 )& 0xFF;
					dev->can_ops->transmit(dev->can_ops->obj, msg_buf, 8);
					#endif
					regs->dcan_nwdat = 0x1;
				}
				regs->if1_cmd &= ~0x8000; /* unset BUSY bit */
			}
			break;
		case 0x110:
			regs->if1_data_a = val;
                        break;
		case 0x114:
			regs->if1_data_b = val;
                        break;
		default:
			printf("Can not write the register at 0x%x in hecc\n", offset);
			return Invarg_exp;
	}
	//printf("In %s, offset=0x%x, val=0x%x\n", __FUNCTION__, offset, val);
	return No_exp;
}
static conf_object_t* new_am35x_hecc(char* obj_name){
	am35x_hecc_device* dev = skyeye_mm_zero(sizeof(am35x_hecc_device));
	hecc_reg_t* regs =  skyeye_mm_zero(sizeof(hecc_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	dev->regs = regs;

	can_ops_intf* ops = skyeye_mm_zero(sizeof(can_ops_intf));
	dev->can_ops = ops;
	ops->obj = NULL;
	ops->start = NULL;
	ops->stop = NULL;
	ops->transmit = NULL;
	ops->receive = NULL;
	SKY_register_interface(dev->can_ops, obj_name, CAN_OPS_INTF_NAME);	
	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = am35x_hecc_read;
	io_memory->write = am35x_hecc_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	

	return dev->obj;
}
void free_am35x_hecc(conf_object_t* dev){
	
}

void init_am35x_hecc(){
	static skyeye_class_t class_data = {
		.class_name = "am35x_hecc",
		.class_desc = "HECC of TI am35x",
		.new_instance = new_am35x_hecc,
		.free_instance = free_am35x_hecc,
		.get_attr = NULL,
		.set_attr = NULL
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}
