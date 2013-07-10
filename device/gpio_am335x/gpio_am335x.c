#include <stdio.h>
#include <skyeye_config.h>
#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h>
#include <memory_space.h>
#include <skyeye_device.h>
#include "skyeye_thread.h"

#include "gpio_am335x.h"



static exception_t am335x_gpio_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	am335x_gpio_dev  *dev = opaque->obj;
	if(!strncmp(attr_name, "signal", strlen("signal"))){
	       dev->signal = (general_signal_intf *)SKY_get_interface(value->u.object, GENERAL_SIGNAL_INTF_NAME);
	}else{
	       printf("parameter error\n");
        return Invarg_exp;
}
return No_exp;
}

static int am335x_gpio_raise_signal(conf_object_t *opaque, int line){
      am335x_gpio_dev *dev = opaque->obj;
      if(line < 0 || line > 32){
	      printf("gpio irq number is not correctly\n");
	      return Not_found_exp;
      }
      if((!(dev->regs.ctrl & 0x1)) && ((dev->regs.irq_set_status0 >> (line - 1)) & 1)){     // the gpio module is enable  and the pinirq is enable
		dev->regs.irqstatus_0 |= (1 << (line - 1));
		dev->signal->raise_signal(dev->signal->conf_obj, 32);
      }
      return No_exp;
}


static int am335x_gpio_lower_signal(conf_object_t *opaque, int line){                                  
 
        return No_exp;
 }




static exception_t am335x_gpio_read(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	am335x_gpio_dev *dev = opaque->obj;
	switch(offset)
	{
		case GPIO_REVISION : 
			*(uint32 *)buf = dev->regs.revision ;
			break;	     
		case GPIO_SYSCONFIG:
			*(uint32 *)buf = dev->regs.sysconfig;
			break;
		case GPIO_IRQSTATUS_RAW_0 : 
			*(uint32 *)buf = dev->regs.irqstatus_raw_0;
			break;	     
		case GPIO_IRQSTATUS_RAW_1 : 
			*(uint32 *)buf = dev->regs.irqstatus_raw_1;
		         break;	     
		case GPIO_IRQSTATUS_0 : 
			*(uint32 *)buf = dev->regs.irqstatus_0;
		         break;	     
		case GPIO_IRQSTATUS_1 : 
			*(uint32 *)buf = dev->regs.irqstatus_1;
		         break;	     
		case GPIO_IRQSTATUS_SET_0 : 
			*(uint32 *)buf = dev->regs.irqstatus_set_0;
		         break;	     
		case GPIO_IRQSTATUS_SET_1 : 
			*(uint32 *)buf = dev->regs.irqstatus_set_1;
		         break;	     
		case GPIO_IRQSTATUS_CLR_0 : 
			*(uint32 *)buf = dev->regs.irqstatus_clr_0;
		         break;	     
		case GPIO_IRQSTATUS_CLR_1 : 
			*(uint32 *)buf = dev->regs.irqstatus_clr_1;
		         break;	     
		case GPIO_IRQWAKEN_0 : 
			*(uint32 *)buf = dev->regs.irqwaken_0;
		         break;	     
		case GPIO_IRQWAKEN_1 : 
			*(uint32 *)buf = dev->regs.irqwaken_1;
		         break;	     
		case GPIO_SYSSTATUS :                                     // R
			*(uint32 *)buf = dev->regs.sysstatus;
		         break;	     
		case GPIO_CTRL : 
			*(uint32 *)buf = dev->regs.ctrl;
		         break;	     
		case GPIO_OE :                                          //R/W  configure pin as input(1) or output(0)
			*(uint32 *)buf = dev->regs.oe;
		         break;	     
		case GPIO_DATAIN : 
			*(uint32 *)buf = dev->regs.datain;
		         break;	     
		case GPIO_DATAOUT : 
			*(uint32 *)buf = dev->regs.dataout;
		         break;	     
		case GPIO_LEVELDETECT0 : 
			*(uint32 *)buf = dev->regs.levedetect0;
		         break;	     
		case GPIO_LEVELDETECT1 : 
			*(uint32 *)buf = dev->regs.levedetect1;
		         break;	     
		case GPIO_RISINGDETECT : 
			*(uint32 *)buf = dev->regs.risingdetect;
		         break;	     
		case GPIO_FALLINGDETECT : 
			*(uint32 *)buf = dev->regs.fallingdetect;
		         break;	     
		case GPIO_DEBOUNCENABLE : 
			*(uint32 *)buf = dev->regs.debouncenable;
		         break;	     
		case GPIO_DEBOUNCINGTIME : 
			*(uint32 *)buf = dev->regs.debouncingtime;
			break;	     
		case GPIO_CLEARDATAOUT: 
			*(uint32 *)buf = dev->regs.cleardataout;
			break;	     
		case GPIO_SETDATAOUT: 
			*(uint32 *)buf = dev->regs.setdataout;
			break;	     
                default:
			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}       

	return  No_exp;
}

static exception_t am335x_gpio_write(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	am335x_gpio_dev *dev = opaque->obj;
	switch(offset)
	{
		case GPIO_REVISION : 
			dev->regs.revision = *(uint32 *)buf;
			break;	     
		case GPIO_SYSCONFIG :                                          // R/W  reset the gpio module register
			dev->regs.sysconfig = *(uint32 *)buf;
			if(dev->regs.sysconfig & 0x2)
				dev->regs.sysstatus |= 0x1;
			break;
		case GPIO_IRQSTATUS_RAW_0 : 
			dev->regs.irqstatus_raw_0 = *(uint32 *)buf;
			break;	     
		case GPIO_IRQSTATUS_RAW_1 : 
			dev->regs.irqstatus_raw_1 = *(uint32 *)buf;
		         break;	     
		case GPIO_IRQSTATUS_0 : 
			dev->regs.irqstatus_0 = *(uint32 *)buf;
		         break;	     
		case GPIO_IRQSTATUS_1 : 
			dev->regs.irqstatus_1 = *(uint32 *)buf;
		         break;	     
		case GPIO_IRQSTATUS_SET_0 : 
			dev->regs.irqstatus_set_0 = *(uint32 *)buf;
			dev->regs.irq_set_status0 |= dev->regs.irqstatus_set_0;
		         break;	     
		case GPIO_IRQSTATUS_SET_1 : 
			dev->regs.irqstatus_set_1 = *(uint32 *)buf;
			dev->regs.irq_set_status1 |= dev->regs.irqstatus_set_1;
		         break;	     
		case GPIO_IRQSTATUS_CLR_0 : 
			dev->regs.irqstatus_clr_0 = *(uint32 *)buf;
			dev->regs.irq_set_status0 &= (~dev->regs.irqstatus_clr_0);
		         break;	     
		case GPIO_IRQSTATUS_CLR_1 : 
			dev->regs.irqstatus_clr_1 = *(uint32 *)buf;
			dev->regs.irq_set_status1 &= dev->regs.irqstatus_clr_1;
		         break;	     
		case GPIO_IRQWAKEN_0 : 
			dev->regs.irqwaken_0 = *(uint32 *)buf;
		         break;	     
		case GPIO_IRQWAKEN_1 : 
			dev->regs.irqwaken_0 = *(uint32 *)buf;
		         break;	     
		case GPIO_CTRL : 
			dev->regs.ctrl = *(uint32 *)buf;
		         break;	     
		case GPIO_OE :                              //R/W  configure pin as input(1) or output(0)
			dev->regs.oe = *(uint32 *)buf;
		         break;	     
		case GPIO_DATAIN : 
			dev->regs.datain = *(uint32 *)buf;
		         break;	     
		case GPIO_DATAOUT : 
			dev->regs.dataout = *(uint32 *)buf;
		         break;	     
		case GPIO_LEVELDETECT0 : 
			dev->regs.levedetect0 = *(uint32 *)buf;
		         break;	     
		case GPIO_LEVELDETECT1 : 
			dev->regs.levedetect1 = *(uint32 *)buf;
		         break;	     
		case GPIO_RISINGDETECT : 
			dev->regs.risingdetect = *(uint32 *)buf;
		         break;	     
		case GPIO_FALLINGDETECT : 
			dev->regs.fallingdetect = *(uint32 *)buf;
		         break;	     
		case GPIO_DEBOUNCENABLE : 
			dev->regs.debouncenable = *(uint32 *)buf;
		         break;	     
		case GPIO_DEBOUNCINGTIME : 
			dev->regs.debouncingtime = *(uint32 *)buf;
			break;	     
		case GPIO_CLEARDATAOUT: 
			dev->regs.cleardataout = *(uint32 *)buf;
			break;	     
		case GPIO_SETDATAOUT: 
			dev->regs.setdataout = *(uint32 *)buf;
			break;	     

		default:
			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}
	return  No_exp;
}

static conf_object_t* new_am335x_gpio(char* obj_name)
{
	am335x_gpio_dev* dev = skyeye_mm_zero(sizeof(am335x_gpio_dev));

	dev->obj = new_conf_object(obj_name, dev);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = dev->obj;
	io_memory->read = am335x_gpio_read;
	io_memory->write = am335x_gpio_write;

	SKY_register_interface((void*)io_memory, obj_name, MEMORY_SPACE_INTF_NAME);

	general_signal_intf *signal = skyeye_mm_zero(sizeof(general_signal_intf));
	signal->conf_obj = dev->obj;
	signal->raise_signal = am335x_gpio_raise_signal;
	signal->lower_signal = am335x_gpio_lower_signal;
	SKY_register_interface((void*)signal, obj_name, GENERAL_SIGNAL_INTF_NAME);

	return dev->obj;
}

static exception_t reset_am335x_gpio(conf_object_t* opaque, const char* args)
{
	am335x_gpio_dev *dev = opaque->obj;
	memset(&(dev->regs), 0, sizeof(dev->regs));
	dev->regs.oe = 0xFFFFFFFF;
	
	return No_exp;        
}


static exception_t free_am335x_gpio(conf_object_t* conf_obj)
{

	return No_exp;
}

void init_am335x_gpio(){
	static skyeye_class_t class_data = {
		.class_name = "am335x_gpio",
		.class_desc = "am335x_gpio",
		.new_instance = new_am335x_gpio,
		.reset_instance = reset_am335x_gpio,
		.free_instance = free_am335x_gpio,
		.get_attr = NULL,
		.set_attr = am335x_gpio_set_attr
	};

	SKY_register_class(class_data.class_name, &class_data);
}

