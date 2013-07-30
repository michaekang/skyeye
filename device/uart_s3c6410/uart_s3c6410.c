/* Copyright (C) 
* 
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
* @file uart_s3c6410.c
* @brief The implementation of system controller
* @author Kewei Yu keweihk@gmail.com
* @version 78.77
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
#include <skyeye_uart_ops.h>
#define DEBUG
#include <skyeye_log.h>

#include "uart_s3c6410.h"

static char* s3c6410_uart_attr[] = {"term", "signal"};

void uart_do_cycle(conf_object_t* opaque)
{
	struct uart_s3c6410_device *dev = opaque->obj;
	uart_s3c6410_reg_t* regs = dev->regs;
	exception_t ret = 0;

	if (((regs->utrstat & 0x1) == 0x0) && ((regs->ucon & 0x3) == 0x1)) {
		struct timeval tv;
		unsigned char buf;

		tv.tv_sec = 0;
		tv.tv_usec = 0;

		if ((ret = (dev->term->read(dev->term->conf_obj, &buf, 1)) == No_exp)){
			/* convert ctrl+c to ctrl+a. */
			if (buf == 1) buf = 3;
			regs->urxh = buf;

			/* Receiver Ready
			 * */
			regs->ufstat |= (0x1); /* 2007-02-09 by Anthony Lee : for 1 bytes */

			/* pending usart0 interrupt
			 * */
			regs->uintp |= (0x1 & ~(regs->uintm));
			regs->uintsp |= 0x1;
			if (regs->uintp) {
				dev->signal->raise_signal(dev->signal->conf_obj, dev->sig_no);
			}
		}
	}
}

static exception_t s3c6410_uart_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	struct uart_s3c6410_device *dev = opaque->obj;
	uart_s3c6410_reg_t* regs = dev->regs;
        int index;
        //parse the parameter
        for(index = 0; index < sizeof(s3c6410_uart_attr)/sizeof(s3c6410_uart_attr[0]);
		index++){
                if(!strncmp(attr_name, s3c6410_uart_attr[index],
			 strlen(s3c6410_uart_attr[index])))
                        break;
        }
        switch(index){
                case 0:
			dev->term = (skyeye_uart_intf*)SKY_get_interface(value->u.object,
					SKYEYE_UART_INTF);
			int  pthread_id;
			create_thread_scheduler(1000, Periodic_sched, uart_do_cycle,
				       	dev->obj, &pthread_id);

                        break;
                case 1:
			dev->signal = (general_signal_intf*)SKY_get_interface(value->u.object,
				GENERAL_SIGNAL_INTF_NAME);
			break;
                default:
                        printf("parameter error\n");
                        return Invarg_exp;
        }
        return No_exp;
}

static exception_t uart_s3c6410_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	struct uart_s3c6410_device *dev = opaque->obj;
	uart_s3c6410_reg_t* regs = dev->regs;
	switch(offset) {
		case ULCON:
			*(uint32_t*)buf = regs->ulcon;
			break;
		case UCON:
			*(uint32_t*)buf = regs->ucon;
			break;
		case UFCON:
			*(uint32_t*)buf = regs->ufcon;
			break;
		case UMCON:
			*(uint32_t*)buf = regs->umcon;
			break;
		case UTRSTAT:
			*(uint32_t*)buf = regs->utrstat;
			break;
		case UERSTAT:
			*(uint32_t*)buf = regs->uerstat;
			break;
		case UFSTAT:
			*(uint32_t*)buf = regs->ufstat;
			break;
		case UMSTAT:
			*(uint32_t*)buf = regs->umstat;
			break;
		case URXH:
			/* receive char
			 * */
			*(uint32_t*)buf = regs->urxh;
			regs->utrstat &= (~0x1);	/* clear strstat register bit[0] */
			regs->ufstat &= ~(0x1); /* 2007-02-09 by Anthony Lee : for 0 bytes */
			break;
		case UBRDIV:
			*(uint32_t*)buf = regs->ubrdiv;
			break;
		case UDIVSLOT:
			*(uint32_t*)buf = regs->ubrdivslot;
			break;
		case UINTP:
			*(uint32_t*)buf = regs->uintp;
			break;
		case UINTSP:
			*(uint32_t*)buf = regs->uintsp;
			break;
		case UINTM:
			*(uint32_t*)buf = regs->uintm;
			break;
		default:
			printf("Can not read the register at 0x%x in uart_s3c6410\n", offset);
			return Invarg_exp;
	}

//printf("In %s offset 0x%x buf 0x%x\n", __func__, offset, *(uint32_t*)buf);
	return No_exp;

}

static exception_t uart_s3c6410_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
//	printf("In %s offset 0x%x buf 0x%x\n", __func__, offset, *buf);
	struct uart_s3c6410_device *dev = opaque->obj;
	uart_s3c6410_reg_t* regs = dev->regs;
	uint32_t data = *buf;
	switch(offset) {
		case ULCON:
			regs->ulcon = data;
			break;
		case UCON:
			regs->ucon = data;
			break;
		case UFCON:
			regs->ufcon = data;
			break;
		case UMCON:
			regs->umcon = data;
			break;
		case UMSTAT:
			regs->umstat = data;
			break;
		case UTXH:
			{
				char c = data;
				/* 2007-01-18 modified by Anthony Lee : for new uart device frame */
				dev->term->write(dev->term->conf_obj, &c, 1);
				regs->utrstat |= 0x6;	/* set strstat register bit[0] */
				regs->ufstat &= 0xff;	/* set strstat register bit[0] */

				regs->uintp |= (0x4 & ~(regs->uintm));
				regs->uintsp |= 0x4;
				dev->signal->raise_signal(dev->signal->conf_obj, dev->sig_no);
			}
			break;
		case UBRDIV:
			regs->ubrdiv = data;
			break;
		case UDIVSLOT:
			regs->ubrdivslot = data;
			break;
		case UINTP:
			regs->uintp &= ~data;
			break;
		case UINTSP:
			regs->uintsp &= ~data;
			break;
		case UINTM:
			regs->uintm = data;
			{
				regs->uintp |= (0x4 & ~(regs->uintm));
				regs->uintsp |= 0x4;
				dev->signal->raise_signal(dev->signal->conf_obj, dev->sig_no);
			}
			break;

		default:
			printf("Can not write the register at 0x%x in uart_s3c6410\n", offset);
			return Invarg_exp;
	}
	return No_exp;
}

static conf_object_t* new_uart_s3c6410(char* obj_name){
	uart_s3c6410_device* dev = skyeye_mm_zero(sizeof(uart_s3c6410_device));
	uart_s3c6410_reg_t* regs =  skyeye_mm_zero(sizeof(uart_s3c6410_reg_t));
	dev->obj = new_conf_object(obj_name, dev);
	/* init uart_s3c6410 regs */
	dev->regs = regs;

	/* Register io function to the object */
	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = uart_s3c6410_read;
	io_memory->write = uart_s3c6410_write;
	SKY_register_interface(io_memory, obj_name, MEMORY_SPACE_INTF_NAME);	

	return dev->obj;
}

#define UART_UTRSTAT_INIT	0x6
exception_t reset_uart_s3c6410(conf_object_t* opaque, const char* parameters){
	struct uart_s3c6410_device *dev = opaque->obj;
	uart_s3c6410_reg_t* regs = dev->regs; 
	
	regs->utrstat = UART_UTRSTAT_INIT;
	dev->sig_no = 37;
	return No_exp;
}

exception_t free_uart_s3c6410(conf_object_t* dev){
	return No_exp;
}

void init_uart_s3c6410(){
	static skyeye_class_t class_data = {
		.class_name = "uart_s3c6410",
		.class_desc = "uart_s3c6410",
		.new_instance = new_uart_s3c6410,
		.free_instance = free_uart_s3c6410,
		.reset_instance = reset_uart_s3c6410,
		.get_attr = NULL,
		.set_attr = s3c6410_uart_set_attr
	};

	SKY_register_class(class_data.class_name, &class_data);
}
