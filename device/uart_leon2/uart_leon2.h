/*
 * =====================================================================================
 *
 *       Filename:  leon2_uart.h
 *
 *    Description:  UART variables
 *
 *        Version:  1.0
 *        Created:  20/05/08 15:53:23
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#ifndef _LEON2_UART_H_
#define _LEON2_UART_H_

/*-----------------------------------------------------------------------------
 *  UART variables
 *-----------------------------------------------------------------------------*/
typedef struct reg_status
{
	uint8 data_ready                          :1;
	uint8 transmitter_shift_register_empty    :1;
	uint8 transmitter_hold_register_empty     :1;
	uint8 break_received                      :1;
	uint8 overrun                             :1;
	uint8 parity_error                        :1;
	uint8 framing_error                       :1;
	uint32 reserved                            :25;
}status_t;

typedef struct reg_control
{
	uint8 receiver_enable                     :1;
	uint8 transmitter_enable                  :1;
	uint8 receiver_interrupt_enable           :1;
	uint8 transmitter_interrupt_enable        :1;
	uint8 parity_select                       :1;
	uint8 parity_enable                       :1;
	uint8 flow_control                        :1;
	uint8 loop_back                           :1;
	uint8 external_clock                      :1;
	uint32 reserved                            :23;
}control_t;



typedef struct UARTState
{
	conf_object_t *obj;
	struct registers
	{
		uint32 data;

		union{
			status_t flag;
			uint32 value;
		}status;

		union{
			control_t flag;
			uint32 value;
		}control;

		uint32 scaler;
	}regs;
	skyeye_uart_intf* term;
}leon2_uart_dev;

#define DBG_LEON2_U	0
#define DBG_LEON2_UART(fmt, ...)      if(DBG_LEON2_U){					\
	                                        printf(fmt, ## __VA_ARGS__);	\
	                                        }
#define UART_DATA_REGISTER      0x0
#define UART_STATUS_REGISTER    0x4
#define UART_CONTROL_REGISTER   0x8
#define UART_SCALER_REGISTER    0xc

#define UART_NREGS  4


#endif


