#ifndef __UART_XDHT_H
#define __UART_XDHT_H

/*************************************/
/* XDHT INTERRUPT CONTROL REGISTERS*/
/*************************************/
#define  BP2007ADD          ((unsigned short *)0x1000000)        
#define  UART_MAIN_BASE     (BP2007ADD + 0x3500)
#define  UART_VIEW_BASE     (BP2007ADD + 0x3510) 
#define  UART_GEN3_BASE     (BP2007ADD + 0x3520) 
#define  UART_MEN4_BASE     (BP2007ADD + 0x3530)
#define  UART_PRM1_BASE     (BP2007ADD + 0x3540)    
#define  UART_PRM2_BASE     (BP2007ADD + 0x3550)
#define  UART_RDIC_BASE     (BP2007ADD + 0x3560)
#define  UART_RNIC_BASE     (BP2007ADD + 0x3570)

/****************/
/*    offset    */
/****************/
#define UARTRFR			0x0			//接收FIFO寄存器		R
#define UARTTFR			0x0			//发送FIFO寄存器		W
#define UARTISR			0x2			//中断设置寄存器		R/W
#define UARTLCR			0x4			//线控制寄存器			R/W
#define UARTIFR			0x6			//中断标志寄存器		R
#define UARTMRR			0x6			//复位寄存器			W
#define UARTRFOR		0x8			//接收FIFO状态寄存器		R
#define UARTRFTLR		0x8			//接收FIFO中断触发深度寄存器	W(必须小于256)
#define UARTTFOR		0xA			//发送FIFO状态寄存器		R
#define UARTCFR			0xA			//FIFO清除寄存器		W
#define UARTDLL			0xC			//波特率分频低位		R/W
#define UARTDLM			0xE			//波特率分频高位		R/W

//中断设置寄存器
#define UARTIE_NONE		0x00		//所有中断关闭
#define UARTRIE			0x01		//接收中断使能
#define UARTTIE			0x02		//发送中断使能
#define UARTTITC		0x04		//发送中断触发条件(0->TX FIFO空，1->TX FIFO空并且发送移位寄存器空)

#define UARTMRE			0x80		//UART模块正常状态	

//中断标志寄存器
#define UARTRIF			0x01		//接收中断标志
#define UARTTIF			0x02		//发送中断标志

//清除FIFO寄存器
#define UARTCRF			0x01		//清除接收FIFO
#define UARTCTF			0x02		//清除发送FIFO




#define READ_BIT(m, reg) ( (reg>>m)&1 )  /*read bit one of register*/
#define SET_BIT(m, reg, n)  ( n ? (reg | (1 << m)):(reg & (~(1 << m))) )  /*set m bit into n of register*/
#define READ_BITS(m, n, reg) (  reg << (15 - (n)) >> ((15 - (n)) + (m))  ) /*read m bit to n of register*/


typedef struct _FIFO
{
    uint8    *pFirst;
    uint8    *pLast; 
    uint8    *pIn;
    uint8    *pOut;
    uint32  Length;
    uint32  Enteres;

}FIFO;



typedef struct UARTState
{
	conf_object_t *obj;
	FIFO rx_fifo;
	FIFO tx_fifo;
	struct registers
	{	
		/**************/
		uint8 uart_rfr;     //接收FIFO寄存器			R
		uint8 uart_tfr;     //发送FIFO寄存器			W
		/**************/
		uint8 uart_isr;     //中断设置寄存器			R/W
		/**************/
		uint8 uart_lcr;     //线控制寄存器			R/W
		/**************/
		uint8 uart_ifr;     //中断标志寄存器			R
		uint8 uart_mrr;     //复位寄存器			W
		/**************/
		uint8 uart_rfor;    //接收FIFO状态寄存器		R
		uint8 uart_rftlr;   //接收FIFO中断触发深度寄存器	W(必须小于256)
		/**************/
		uint8 uart_tfor;    //发送FIFO状态寄存器		R
		uint8 uart_cfr;     //FIFO清除寄存器			W
		/**************/
		uint8 uart_dll;     //波特率分频低位			R/W
		/**************/
		uint8 uart_dlm;     //波特率分频高位			R/W
		/**************/
	}regs;
	skyeye_uart_intf* term;
	general_signal_intf *signal;
}xdht_uart_dev;


#endif
