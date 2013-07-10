#include <stdio.h>
#include <skyeye_config.h>
#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_exec.h>
#include <skyeye_cell.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h>
#include <memory_space.h>
#include <skyeye_device.h>
#include "skyeye_uart.h"
#include "skyeye_thread.h"

#include "uart_xdht.h"


void CreateFIFO(FIFO *fifo, uint32 FIFOLength)
{
    uint8 *pfifo;
    pfifo = (uint8 *)malloc(FIFOLength);
    fifo->pFirst = pfifo;
    fifo->pLast = pfifo + FIFOLength-1;
    fifo->Length = FIFOLength;
    fifo->pIn     = fifo->pFirst;
    fifo->pOut    = fifo->pFirst;
    fifo->Enteres = 0;  
}

uint32 WriteFIFO(FIFO *fifo, uint8 *pSource,uint32 WriteLength)
{
    uint32 i;
    
	for (i = 0; i < WriteLength; i++){
		if (fifo->Enteres >= fifo->Length){
			return i;//如果已经写入FIFO的数据两超过或者等于FIFO的长度，就返回实际写入FIFO的数据个数
		}
		*(fifo->pIn ++ ) = *(pSource ++);
		if (fifo->pIn == fifo->pLast){
			fifo->pIn = fifo->pFirst;
		}
		fifo->Enteres ++;
	}
    return i;
}

uint32 ReadFIFO(FIFO *fifo, uint8 *pAim,uint32 ReadLength)
{
    uint32 i;
	for (i = 0; i < ReadLength; i++){
		if (fifo->Enteres <= 0){
			return i;//返回从FIFO中读取到的数据个数
		}
		*(pAim ++) = *(fifo->pOut ++);
		if (fifo->pOut == fifo->pLast){
			fifo->pOut = fifo->pFirst;
		}
		fifo->Enteres -- ;
	} 
	return i;
}

uint32 CheckFIFOLength(FIFO *fifo)
{
    return fifo->Length;
}


uint8* CheckCurrentWritePoint(FIFO *fifo)
{
    return (fifo->pIn);
}

uint8* CheckCurrentReadPoint(FIFO *fifo)
{
    return (fifo->pOut);
}


void FreeFIFO(FIFO *fifo)
{
    free(fifo->pFirst);
}

uint32 CheckCanWriteNum(FIFO *fifo)
{
    return (fifo->Length - fifo->Enteres);
}

uint32 CheckCanReadNum(FIFO *fifo)
{
    return fifo->Enteres;
}

static void *hardware_trans(void *uart_dev)
{
	xdht_uart_dev* dev  = uart_dev;
	uint8 buf, rxbuf;
	int ret;
	while(!dev->term | !dev);
	while(1){
		ret = dev->term->read(dev->term->conf_obj, &buf, 1);
		if(ret == No_exp){
			WriteFIFO(&dev->rx_fifo, &buf, 1);
			ret = CheckCanReadNum(&dev->rx_fifo);
			dev->regs.uart_rfor = ret;
			if(ret >= dev->regs.uart_rftlr && (dev->regs.uart_isr & UARTRIE == UARTRIE)){
				dev->regs.uart_ifr |= UARTRIF;
				dev->signal->raise_signal(dev->signal->conf_obj, 384);/*触发接收中断*/
			}
		}
	}

	return NULL;
}


static exception_t xdht_uart_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	xdht_uart_dev  *dev = opaque->obj;
	if(!strncmp(attr_name, "term", strlen("term"))){
		dev->term = (skyeye_uart_intf *)SKY_get_interface(value->u.object, SKYEYE_UART_INTF);
	}else if(!strncmp(attr_name, "signal", strlen("signal"))){
		dev->signal = (general_signal_intf *)SKY_get_interface(value->u.object, GENERAL_SIGNAL_INTF_NAME);
	}else{
		printf("parameter error\n");
		return Invarg_exp;
	}
	return No_exp;
}

static exception_t xdht_uart_read(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	xdht_uart_dev *dev = opaque->obj;
	switch(offset)
	{
	        case UARTRFR:        //接收FIFO寄存器		R
			ReadFIFO(&dev->rx_fifo, buf, 1);
			dev->regs.uart_rfor =  CheckCanReadNum(&dev->rx_fifo);
	                break;       
		case UARTISR:        //中断设置寄存器		R/W
			*(uint8 *)buf = dev->regs.uart_isr;
	                break;       
                case UARTLCR:        //线控制寄存器		R/W
			*(uint8 *)buf = dev->regs.uart_lcr;
                        break;       
                case UARTIFR:        //中断标志寄存器		R
			*(uint8 *)buf = dev->regs.uart_ifr;
			dev->regs.uart_ifr = 0x0;
                        break;        
		case UARTRFOR:       //接收FIFO状态寄存器	R
			*(uint8 *)buf = dev->regs.uart_rfor;
	                break;       
		case UARTTFOR:       //发送FIFO状态寄存器	R
			*(uint8 *)buf = dev->regs.uart_tfor;
	                break;       
		case UARTDLL:        //波特率分频低位		R/W
			*(uint8 *)buf = dev->regs.uart_dll;
	                break;
		case UARTDLM:        //波特率分频高位		R/W
			*(uint8 *)buf = dev->regs.uart_dlm;
	                break;
                default:

			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}       

	return  No_exp;
}

static exception_t xdht_uart_write(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	xdht_uart_dev *dev = opaque->obj;
	skyeye_uart_intf* skyeye_uart = dev->term;
	switch(offset)
	{
		case UARTTFR:        //发送FIFO寄存器			W
			skyeye_uart->write(skyeye_uart->conf_obj, buf, 1);
			if(dev->regs.uart_isr & UARTTIE){
				dev->regs.uart_ifr |= UARTTIF;
			    	dev->signal->raise_signal(dev->signal->conf_obj, 384);                        /*触发中断*/
			}
	                break;       
		case UARTISR:        //中断设置寄存器			R/W
			dev->regs.uart_isr = *(uint8 *)buf;
			if((dev->regs.uart_isr & UARTRIE) | (dev->regs.uart_isr & UARTTIE)){
				if(dev->regs.uart_isr & UARTTIE){
					dev->regs.uart_ifr |= UARTTIF;
			    	        dev->signal->raise_signal(dev->signal->conf_obj, 384);                        /*触发中断*/
				}
								}
	                break;       
                case UARTLCR:        //线控制寄存器			R/W
			dev->regs.uart_isr = *(uint8 *)buf;
                        break;       
                case UARTMRR:        //复位寄存器			W
			dev->regs.uart_mrr = *(uint8 *)buf;
                        break;        
		case UARTRFTLR:      //接收FIFO中断触发深度寄存器	W(必须小于256)	W
			dev->regs.uart_rftlr = *(uint8 *)buf;
	                break;       
		case UARTCFR:        //FIFO清除寄存器			W
			dev->regs.uart_cfr = *(uint8 *)buf;
	                break;       
		case UARTDLL:        //波特率分频低位			R/W
			dev->regs.uart_dll = *(uint8 *)buf;
	                break;
		case UARTDLM:        //波特率分频高位			R/W
			dev->regs.uart_dlm = *(uint8 *)buf;
			break;
		default:
			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}
	return  No_exp;
}

static conf_object_t* new_xdht_uart(char* obj_name)
{
	xdht_uart_dev* xdht_uart = skyeye_mm_zero(sizeof(xdht_uart_dev));

	xdht_uart->obj = new_conf_object(obj_name, xdht_uart);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = xdht_uart->obj;
	io_memory->read = xdht_uart_read;
	io_memory->write = xdht_uart_write;

	SKY_register_interface((void*)io_memory, obj_name, MEMORY_SPACE_INTF_NAME);


	int  pthread_id;
	create_thread_scheduler(1, Periodic_sched, hardware_trans, xdht_uart, &pthread_id);
	
	return xdht_uart->obj;
}

static exception_t reset_xdht_uart(conf_object_t* opaque, const char* args)
{
#if 1
	xdht_uart_dev *dev = opaque->obj;
	memset(&(dev->regs), 0, sizeof(dev->regs));
	CreateFIFO(&dev->rx_fifo, 16);
	dev->term = NULL;
	
#endif
	return No_exp;        
}


static exception_t free_xdht_uart(conf_object_t* conf_obj)
{

	return No_exp;
}

void init_xdht_uart(){
	static skyeye_class_t class_data = {
		.class_name = "xdht_uart",
		.class_desc = "xdht_uart",
		.new_instance = new_xdht_uart,
		.reset_instance = reset_xdht_uart,
		.free_instance = free_xdht_uart,
		.get_attr = NULL,
		.set_attr = xdht_uart_set_attr
	};

	SKY_register_class(class_data.class_name, &class_data);
}

