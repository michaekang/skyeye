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
	skyeye_uart_intf* skyeye_uart = dev->skyeye_uart->conf_obj;
	DBG_LEON2_UART("write %s offset %d : 0x%x\n", dev->obj->objname, offset, *buf);

	switch(offset)
	{
		case UART_DATA_REGISTER:

			/*  Write the information in the DATA register  */
			dev->regs.data = *buf;

			/*  Indicate that the transmitter hold register is NOT empty    */
			dev->regs.status.flag.transmitter_hold_register_empty = 0;
			skyeye_uart->write(skyeye_uart->conf_obj, buf, 1);

			break;
		case UART_STATUS_REGISTER:
			// do nothing
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

	if(offset == UART_DATA_REGISTER)
		dev->regs.status.flag.data_ready = 0;

	DBG_LEON2_UART("read leon2 uart %s offset %d : 0x%x\n", dev->obj->objname, offset, *(uint32*)buf);

	return No_exp;
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
	/*  Clear the UART register   */
	leon2_uart->obj = new_conf_object(obj_name, leon2_uart);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = leon2_uart->obj;
	io_memory->read = leon2_uart_read;
	io_memory->write = leon2_uart_write;
	SKY_register_interface((void*)io_memory, obj_name, MEMORY_SPACE_INTF_NAME);
	/* register skyeye_uart interface */
	skyeye_intf_t* skyeye_uart = skyeye_mm_zero(sizeof(skyeye_intf_t));
	skyeye_uart->intf_name = SKYEYE_UART_INTF;
	skyeye_uart->class_name = "leon2_uart";
	skyeye_uart->registered = 0;
	skyeye_uart->obj = NULL;
	SKY_register_interface(skyeye_uart, obj_name, SKYEYE_UART_INTF);
	leon2_uart->skyeye_uart = skyeye_uart;

	DBG_LEON2_UART("In %s, Line %d, create leon2 uart\n", __func__, __LINE__);

	return leon2_uart->obj;
}

static exception_t reset_leon2_uart(conf_object_t* opaque, const char* args)
{
	leon2_uart_dev *dev = opaque->obj;
	memset(&(dev->regs), 0, sizeof(dev->regs));
	/*  Enable the transmitter and receiver by default  */
	dev->regs.control.flag.transmitter_enable = 1;
	dev->regs.control.flag.receiver_enable = 1;

	/*  Indicate that the transmitter hold register is EMPTY    */
	dev->regs.status.flag.transmitter_hold_register_empty = 1;
	dev->regs.status.flag.transmitter_shift_register_empty = 1;

	return No_exp;
}

static void free_leon2_uart(conf_object_t* dev)
{
	printf("In %s Line %d\n", __func__, __LINE__);
}

void init_leon2_uart(){
	static skyeye_class_t class_data = {
		.class_name = "leon2_uart",
		.class_desc = "leon2 uart",
		.new_instance = new_leon2_uart,
		.reset_instance = reset_leon2_uart,
		.free_instance = free_leon2_uart,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}

