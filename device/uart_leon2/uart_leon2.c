/*
 * =====================================================================================
 *
 *       Filename:  leon2_uart.c
 *
 *    Description:  This file implements the LEON2 UART on-chip device.
 *
 *        Version:  1.0
 *        Created:  24/06/08 10:51:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

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
#include "skyeye_uart.h"

#include "uart_leon2.h"

static char* leon2_uart_attr[] = {"term"};
static exception_t leon2_uart_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	leon2_uart_dev *dev = opaque->obj;
	int index;
	//parse the parameter
	for(index = 0; index < 3; index++){
		if(!strncmp(attr_name, leon2_uart_attr[index], strlen(leon2_uart_attr[index])))
			break;
	}
	switch(index){
		case 0:
			dev->term = (skyeye_uart_intf*)SKY_get_interface(value->u.object, SKYEYE_UART_INTF);
			break;
		default:
			printf("parameter error\n");
			return Invarg_exp;
	}
	return No_exp;
}
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_cycle
 *  Description:  This function performs the execution of the UART cycle. The
 *  function checks whether there is some to transmit from the UART.
 *
 *  @TODO: The implementation is not finished. Still the implementation does not
 *  care about the possible generated interrupts.
 * =====================================================================================
 */
void leon2_uart_cycle(conf_object_t *opaque)
{
	leon2_uart_dev *dev = opaque->obj;
	/*-----------------------------------------------------------------------------
	 *  TRANSMITER OPERATION
	 *-----------------------------------------------------------------------------*/
	/*  Check whether there is something to transmit from the UART. If
	 *  transmitter is enabled through the TE bit in the control register, the
	 *  data is transmiter by means of the skyeye uart  */
	if(dev->regs.control.flag.transmitter_enable )
	{
		if( !dev->regs.status.flag.transmitter_hold_register_empty )
		{
			char c = dev->regs.data;

			/*  Call the SKYEYE uart function to print out the character    */
			skyeye_uart_write(-1, &c, 1, NULL);

			/*  clear the transmitter data register */
			//            clear_bits(regmap.uart_1_data, LEON2_UART_DATA_last, LEON2_UART_DATA_first);
			/*  Indicate the transmiter hold register is empty  */
			dev->regs.status.flag.transmitter_hold_register_empty = 1;
		}
	}
	else
	{
		/*  UART Transmiter not enabled    */
		DBG_LEON2_UART("Not enable %s\n", dev->obj->objname);
	}
	/*-----------------------------------------------------------------------------
	 *  RECEIVER OPERATION
	 *-----------------------------------------------------------------------------*/
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_write
 *  Description:  This function perform the writing in the UART device
 * =====================================================================================
 */
static exception_t leon2_uart_write(conf_object_t *opaque, generic_address_t offset, uint32_t* buf, size_t count)
{
	leon2_uart_dev *dev = opaque->obj;
	skyeye_uart_intf* skyeye_uart = dev->term;
	DBG_LEON2_UART("write %s offset %d : 0x%x\n", dev->obj->objname, offset, *buf);
	char* name = NULL;
	uint32_t data = 0;

	switch(offset)
	{
		case UART_DATA_REGISTER:
			/*  Write the information in the DATA register  */
			dev->regs.data = *buf;
			/*  Indicate that the transmitter hold register is NOT empty    */
			if(dev->regs.control.flag.transmitter_enable == 1){
				skyeye_uart->write(skyeye_uart->conf_obj, buf, 1);
				dev->regs.status.flag.transmitter_hold_register_empty = 1;
			}else{
				printf("Don't enable the uart transmitter!\n");
			}
			break;
		case UART_STATUS_REGISTER:
			// do nothing
			break;
		case UART_CONTROL_REGISTER:
			dev->regs.control.value = *buf;
			break;
		case UART_SCALER_REGISTER:
			break;
		default:
			printf("write %s error offset %d : 0x%x\n",
					dev->obj->objname, offset, *buf);
			break;
	}
	return No_exp;
}

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  leon2_uart_read
 *  Description:  This function performs the reading in the UART device.
 * =====================================================================================
 */
static exception_t leon2_uart_read(conf_object_t *opaque, generic_address_t offset, void* buf, size_t count)
{
	leon2_uart_dev *dev = opaque->obj;

	//*buf = ((uint32 *)(&(dev->regs)))[offset];

	switch(offset)
	{
		case UART_DATA_REGISTER:
			// do nothing
			break;
		case UART_STATUS_REGISTER:
			*(uint32*)buf =dev->regs.status.value;
			break;
		case UART_CONTROL_REGISTER:
			*(uint32*)buf =dev->regs.control.value;
			break;
		case UART_SCALER_REGISTER:
			*(uint32*)buf =dev->regs.scaler;
			break;
		default:
			printf("write %s error offset %d : 0x%x\n",
					dev->obj->objname, offset, *(uint32*)buf);
			break;
	}
	DBG_LEON2_UART("read leon2 uart %s offset %d : 0x%x\n", dev->obj->objname, offset, *(uint32*)buf);

	return No_exp;
}

char* leon2_uart_get_regname_by_id(conf_object_t* conf_obj, uint32_t id)
{
	leon2_uart_dev *dev = conf_obj->obj;
	return dev->regs_name[id];
}

uint32_t leon2_get_regvalue_by_id(conf_object_t* conf_obj, uint32_t id){
	leon2_uart_dev *dev = conf_obj->obj;
	uint32_t* regs_value = (uint32_t*)&(dev->regs) + id;
	return *regs_value;
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  new_leon2_uart
 *  Description:  instance leon2_uart.
 *
 *  The function returns base class of point which point to the leon2_uart.
 * =====================================================================================
 */
static conf_object_t* new_leon2_uart(char* obj_name)
{
	leon2_uart_dev* leon2_uart = skyeye_mm_zero(sizeof(leon2_uart_dev));
	leon2_uart->regs_name = &regs_name;
	/*  Clear the UART register   */
	leon2_uart->obj = new_conf_object(obj_name, leon2_uart);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = leon2_uart->obj;
	io_memory->read = leon2_uart_read;
	io_memory->write = leon2_uart_write;
	SKY_register_interface((void*)io_memory, obj_name, MEMORY_SPACE_INTF_NAME);
	/* register skyeye_uart interface */

	skyeye_reg_intf* reg_intf = skyeye_mm_zero(sizeof(skyeye_reg_intf));
	reg_intf->conf_obj = leon2_uart->obj;
	reg_intf->get_regvalue_by_id = leon2_get_regvalue_by_id;
	reg_intf->get_regname_by_id = leon2_uart_get_regname_by_id;
	reg_intf->set_regvalue_by_id = NULL;
	SKY_register_interface((void*)reg_intf, obj_name, SKYEYE_REG_INTF);
	DBG_LEON2_UART("In %s, Line %d, create leon2 uart\n", __func__, __LINE__);

	return leon2_uart->obj;
}

static exception_t reset_leon2_uart(conf_object_t* opaque, const char* args)
{
	leon2_uart_dev *dev = opaque->obj;
	memset(&(dev->regs), 0, sizeof(dev->regs));
	/*  Enable the transmitter and receiver by default  */
	dev->regs.control.flag.transmitter_enable = 0;
	dev->regs.control.flag.receiver_enable = 0;

	/*  Indicate that the transmitter hold register is EMPTY    */
	dev->regs.status.flag.transmitter_hold_register_empty = 1;
	dev->regs.status.flag.transmitter_shift_register_empty = 1;

	return No_exp;
}

static void free_leon2_uart(conf_object_t* conf_obj)
{
	leon2_uart_dev* dev = conf_obj->obj;
	conf_object_t* conf_term = dev->term->conf_obj;
	if(conf_term == NULL)
		return;
	conf_object_t* class_obj = get_conf_obj(conf_term->class_name);
	skyeye_class_t* class_data = class_obj->obj;
	/* free uart attrabute : term */
	if(class_data->free_instance)
		class_data->free_instance(conf_term);
	skyeye_reg_intf* reg_intf = (skyeye_reg_intf*)SKY_get_interface(dev->obj, SKYEYE_REG_INTF);
	if(reg_intf != NULL){
		/* free reg_intf */
		skyeye_free(reg_intf);
	}
	/* free uart term interface */
	skyeye_free(dev->term);
	skyeye_free(dev->obj);
	skyeye_free(dev);
}

void init_leon2_uart(){
	static skyeye_class_t class_data = {
		.class_name = "leon2_uart",
		.class_desc = "leon2 uart",
		.new_instance = new_leon2_uart,
		.reset_instance = reset_leon2_uart,
		.free_instance = free_leon2_uart,
		.get_attr = NULL,
		.set_attr = leon2_uart_set_attr
	};

	SKY_register_class(class_data.class_name, &class_data);
}
