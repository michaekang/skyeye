#ifndef __UART_AM335X_H
#define __UART_AM335X_H

/*************************************/
/* AM335X INTERRUPT CONTROL REGISTERS*/
/*************************************/
#define  UART0_BASE    (0x44E09000)
#define  UART0_SIZE    (0xFFF)
#define  UART1_BASE    (0x48022000)
#define  UART1_SIZE    (0xFFF)
#define  UART2_BASE    (0x48024000)
#define  UART2_SIZE    (0xFFF)
#define  UART3_BASE    (0x481A6000)
#define  UART3_SIZE    (0xFFF)
#define  UART4_BASE    (0x481A8000)
#define  UART4_SIZE    (0xFFF)
#define  UART5_BASE    (0x481AA000)
#define  UART5_SIZE    (0xFFF)

/****************/
/*    offset    */
/****************/
#define        RHR           (0x0)
#define        THR           (0x0)
#define        IER           (0x4)
#define        IIR           (0x8)
#define        FCR           (0x8)
#define        LCR           (0xC)
#define        MCR           (0x10)
#define        LSR           (0x14)
#define        MSR           (0x18)
#define        SPR           (0x1C)
#define        MDR1          (0x20)
#define        MDR2          (0x24)
#define        SFLSR         (0x28)
#define        RESUME        (0x2C)
#define        SFREGL        (0x30)
#define        SFREGH        (0x34)
#define        BLR           (0x38)
#define        ACREG         (0x3C)
#define        SCR           (0x40)
#define        SSR           (0x44)
#define        EBLR          (0x48)
#define        MVR           (0x50)
#define        SYSC          (0x54)
#define        SYSS          (0x58)
#define        WER           (0x5C)
#define        CFPS          (0x60)
#define        RXFIFO_LVL    (0x64)
#define        TXFIFO_LVL    (0x68)
#define        IER2          (0x6C)
#define        ISR2          (0x70)
#define        FREQ_SEL      (0x74)
#define        MDR3          (0x80)
#define        TXDMA         (0x84)

#define READ_BIT(m, reg) ( (reg>>m)&1 )  /*read bit one of register*/
#define SET_BIT(m, reg, n)  ( n ? (reg | (1 << m)):(reg & (~(1 << m))) )  /*set m bit into n of register*/
#define READ_BITS(m, n, reg) (  reg << (15 - (n)) >> ((15 - (n)) + (m))  ) /*read m bit to n of register*/

/**********check which mode************************/
//#define REG_LCR   dev->regs.lcr
//#define IS_OP_MODE (READ_BIT(7, REG_LCR) == 0)
//#define IS__MODE_A (READ_BIT(7, REG_LCR) == 1 && READ_BITS(0, 7, REG_LCR) != 0xBF)
//#define IS__MODE_B (READ_BIT(7, REG_LCR) == 1 && READ_BITS(0, 7, REG_LCR)  = 0xBF)
#define IS_OP_MODE   (dev->regs.lcr == 0x007F)
#define IS_MODE_A    (dev->regs.lcr == 0x0080)
#define IS_MODE_B    (dev->regs.lcr == 0x00BF)
/**********TCR and TLR registers enable************/
#define TCR_TLR_ENABLE  (READ_BIT(4, dev->regs.efr) == 0x1 && READ_BIT(6, dev->regs.mcr) == 0x1)
/**************************************************/

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
		/***************/
		uint16   rhr;   /* operational mode  read */    
		uint16   thr;   /* operational mode  write */   
		uint16   dll;   /* mode A\B */
		/***************/
		uint16   ier;   /* operational mode */    
		uint16   dlh;   /* mode A\B */
		/***************/
		uint16   iir;   /* operational mode read register\mode A  */    
		uint16   fcr;   /* operational mode write registe\mode A */    
		uint16   efr;   /* mode B */
		/***************/
                uint16   lcr;   /* all mode */     
		/***************/
		uint16   mcr;   /* operational mode\mode A */  
		uint16   xon1_addr1;   /* mode B*/  
		/***************/
                uint16   lsr;   /* operational mode read\mode A */       
		uint16   xon2_addr2;   /* mode B*/  
		/***************/
                uint16   msr;   /* operational mode read\mode A read*/  
                uint16   tcr;   /* all mode */    
                uint16   xoff1; /* mode B*/      
		/***************/
                uint16   spr;   /* operational mode\mode A */      
                uint16   tlr;   /* all mode */      
                uint16   xoff2; /* mode B */      
		/***************/
		uint16   mdr1;  /* all mode */    
		/***************/
		uint16   mdr2;  /* all mode */    
		/***************/
		uint16   sflsr; /* all mode */    
		/***************/
                uint16   resume;/* all mode */   
		/***************/
                uint16   sfregl;/* all mode */  
		/***************/
                uint16   sfregh;/* all mode */ 
		/***************/
                uint16   blr;   /* operational mode */
                uint16   uasr;   /* mode A\modeB */
		/***************/
                uint16   acreg; /* all mode */    
		/***************/
                uint16   scr;   /* all mode */ 
		/***************/
                uint16   ssr;   /* all mode read */  
		/***************/
                uint16   eblr;  /* operation */    
		/***************/
                uint16   mvr;   /* all mode read */    
		/***************/
                uint16   sysc;  /* all mode */    
		/***************/
                uint16   syss;  /* all mode read*/    
		/***************/
                uint16   wer;   /* all mode */    
		/***************/
                uint16   cfps;  /* all mode */    
		/***************/
                uint16   rxfifo_lvl;/* all mode */
		/***************/
                uint16   txfifo_lvl;/* all mode */
		/***************/
                uint16   ier2;  /* all mode */    
		/***************/
                uint16   isr2;  /* all mode */    
		/***************/
                uint16   freo_sel;/* all mode */
		/***************/
                uint16   mdr3;  /* all mode */    
		/***************/
                uint16   txdma; /* all mode*/    
		/***************/
	}regs;
	volatile skyeye_uart_intf* term;
}am335x_uart_dev;


#endif
