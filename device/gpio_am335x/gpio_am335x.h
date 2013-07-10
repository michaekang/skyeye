#ifndef __GPIO_AM335X_H
#define __GPIO_AM335X_H

/*******************************************************/
/*      Base addresses of GPIO memory mapped registers */                               
/*******************************************************/
#define SOC_GPIO_0_REGS                      (0x44E07000) 
#define SOC_GPIO_1_REGS                      (0x4804C000)
#define SOC_GPIO_2_REGS                      (0x481AC000)
#define SOC_GPIO_3_REGS                      (0x481AE000)


/********************************************************/
/*       GPIO REGISTERS OFFSET                          */
/********************************************************/

#define 	GPIO_REVISION                     (0x0)            	
#define         GPIO_SYSCONFIG 		          (0x10) 
#define		GPIO_IRQSTATUS_RAW_0 	          (0x24) 
#define		GPIO_IRQSTATUS_RAW_1 	          (0x28) 
#define		GPIO_IRQSTATUS_0 		  (0x2C) 
#define		GPIO_IRQSTATUS_1 		  (0x30) 
#define		GPIO_IRQSTATUS_SET_0 	          (0x34) 
#define		GPIO_IRQSTATUS_SET_1  	          (0x38) 
#define		GPIO_IRQSTATUS_CLR_0              (0x3C) 
#define		GPIO_IRQSTATUS_CLR_1 	          (0x40) 
#define		GPIO_IRQWAKEN_0 		  (0x44) 
#define		GPIO_IRQWAKEN_1 		  (0x48) 
#define		GPIO_SYSSTATUS 		  	  (0x114)
#define		GPIO_CTRL 			  (0x130)
#define		GPIO_OE 			  (0x134)
#define		GPIO_DATAIN 		          (0x138)
#define		GPIO_DATAOUT 		          (0x13C)
#define		GPIO_LEVELDETECT0 		  (0x140)
#define		GPIO_LEVELDETECT1 		  (0x144)
#define		GPIO_RISINGDETECT 		  (0x148)
#define		GPIO_FALLINGDETECT 	          (0x14C)
#define		GPIO_DEBOUNCENABLE 	          (0x150)
#define		GPIO_DEBOUNCINGTIME 	          (0x154)
#define		GPIO_CLEARDATAOUT 		  (0x190)
#define		GPIO_SETDATAOUT                   (0x194)


#define READ_BIT(m, reg) ( (reg>>m)&1 )  /*read bit one of register*/
#define SET_BIT(m, reg, n)  ( n ? (reg | (1 << m)):(reg & (~(1 << m))) )  /*set m bit into n of register*/
#define READ_BITS(m, n, reg) (  reg << (15 - (n)) >> ((15 - (n)) + (m))  ) /*read m bit to n of register*/






typedef struct GPIOState
{
	conf_object_t *obj;
	struct registers
	{	
		uint32  revision;
		uint32  sysconfig;
		uint32  irqstatus_raw_0;
		uint32  irqstatus_raw_1;
		uint32  irqstatus_0;
		uint32  irqstatus_1;
		uint32  irqstatus_set_0;
		uint32  irqstatus_set_1;
		uint32  irqstatus_clr_0;
		uint32  irqstatus_clr_1;
		uint32  irqwaken_0;
		uint32	irqwaken_1;
		uint32  sysstatus; 
		uint32	ctrl;
		uint32  oe;
		uint32	datain;
		uint32  dataout;
		uint32	levedetect0;
		uint32  levedetect1;
		uint32	risingdetect;
		uint32  fallingdetect;
		uint32	debouncenable;
		uint32  debouncingtime;
		uint32	cleardataout;
		uint32  setdataout;

		/*****************/
		uint32  irq_set_status0;
		uint32  irq_set_status1;
		/*****************/
	}regs;  
	general_signal_intf *signal;
}am335x_gpio_dev;

#endif
