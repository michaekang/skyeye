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
	int data_ready                          :1;
	int transmitter_shift_register_empty    :1;
	int transmitter_hold_register_empty     :1;
	int break_received                      :1;
	int overrun                             :1;
	int parity_error                        :1;
	int framing_error                       :1;
	int reserved                            :25;
}status_t;

typedef struct reg_control
{
	int receiver_enable                     :1;
	int transmitter_enable                  :1;
	int receiver_interrupt_enable           :1;
	int transmitter_interrupt_enable        :1;
	int parity_select                       :1;
	int parity_enable                       :1;
	int flow_control                        :1;
	int loop_back                           :1;
	int external_clock                      :1;
	int reserved                            :23;
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
}leon2_uart_dev;

#define DBG_LEON2_U	1
#define DBG_LEON2_UART(fmt, ...)      if(DBG_LEON2_U){					\
	                                        printf(fmt, ## __VA_ARGS__);	\
	                                        }
#define UART_DATA_REGISTER      0x0
#define UART_STATUS_REGISTER    0x4
#define UART_CONTROL_REGISTER   0x8
#define UART_SCALER_REGISTER    0xc

#define UART_NREGS  4


#endif


