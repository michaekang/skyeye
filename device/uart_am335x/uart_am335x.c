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
#include "skyeye_uart.h"
#include "skyeye_thread.h"

#include "uart_am335x.h"


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
	am335x_uart_dev *dev= uart_dev;
	uint8 buf;
	int ret;
	while(1){
		ret = 0;
		while(!dev->term)
		{
		//	printf("\n");
		}
		//ret = dev->term->read(dev->term->conf_obj, &buf, 1);
	}

	return NULL;
}



static exception_t am335x_uart_set_attr(conf_object_t* opaque, const char* attr_name, attr_value_t* value)
{
	am335x_uart_dev  *dev = opaque->obj;
	if(!strncmp(attr_name, "term", strlen("term"))){
		dev->term = (skyeye_uart_intf *)SKY_get_interface(value->u.object, SKYEYE_UART_INTF);
	}else{
		printf("parameter error\n");
		return Invarg_exp;
	}
	return No_exp;
}

static exception_t am335x_uart_read(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	am335x_uart_dev *dev = opaque->obj;
	switch(offset)
	{
		case RHR: 
			if(IS_OP_MODE){
				*(uint16 *)buf = dev->regs.rhr;
				if(dev->rx_fifo.Enteres == 0)
					SET_BIT(0, dev->regs.lsr, 0);

			}else if(IS_MODE_A){

			}else if(IS_MODE_B){

			}
			//ReadFIFO(&dev->rx_fifo, (uint8 *)&buf, 1);
			break;
		case IER:
			if(IS_OP_MODE){
				*(uint16 *)buf = dev->regs.ier;
			}else if(IS_MODE_A){
				*(uint16 *)buf = dev->regs.dlh;
			}else if(IS_MODE_B){
				*(uint16 *)buf = dev->regs.dlh;
			}
		       	break;
		case IIR:
			if(IS_OP_MODE){
				*(uint16 *)buf = dev->regs.iir;  /*operational mode UART_IIR register read*/
			}else if(IS_MODE_A){
				*(uint16 *)buf = dev->regs.iir;  /*MODE A UART_IIR register read*/
			}else if(IS_MODE_B){
				*(uint16 *)buf = dev->regs.efr;  /*MODE B UART_EFR register read*/
			}
		       	break;
		case LCR:
			*(uint16 *)buf = dev->regs.lcr;
		       	break;
		case MCR:
			*(uint16 *)buf = dev->regs.mcr;
                       	break;
		case LSR:
			*(uint16 *)buf = dev->regs.lsr;
                       	break;
                case MSR:
			if(TCR_TLR_ENABLE){ /*Transmission control register (TCR) are accessible only when EFR[4] = 1 and MCR[6] = 1*/
				*(uint16 *)buf = dev->regs.tcr; /*all mode UART_TCR read*/
			}else{
				if(IS_OP_MODE | IS_MODE_A){  
					*(uint16 *)buf = dev->regs.msr; /*operational mode and MODE A UART_MSR read*/
				}else if(IS_MODE_B){
					*(uint16 *)buf = dev->regs.xoff1; /*MODE B UART_XOFF1 read*/
				}
			}
                       	break;
		case SPR:
			if(TCR_TLR_ENABLE){ /*trigger level register (TLR) are accessible only when EFR[4] = 1 and MCR[6] = 1*/
				*(uint16 *)buf = dev->regs.tlr;  /*all mode UART_TLR read*/
			}else{
				if(IS_OP_MODE | IS_MODE_A){
					*(uint16 *)buf = dev->regs.spr; /*operational mode and MODE A UART_SPR read*/
				}else if(IS_MODE_B){
					*(uint16 *)buf = dev->regs.xoff2;/*MODE B UART_XOFF2 read*/
				}
			}
                       	break;
                case MDR1:
			*(uint16 *)buf = dev->regs.mdr1;
                       	break;
                case MDR2:
			*(uint16 *)buf = dev->regs.mdr2;
                       	break;
                case SFLSR:
			*(uint16 *)buf = dev->regs.sflsr;
                       	break;
		case RESUME:
			*(uint16 *)buf = dev->regs.resume;
	               	break;
                case SFREGL:
			*(uint16 *)buf = dev->regs.sfregl;
                       	break;
                case SFREGH:
			*(uint16 *)buf = dev->regs.sfregh;
                       	break;
                case BLR:
			*(uint16 *)buf = dev->regs.sfregh;
                       	break;
                case ACREG:
			*(uint16 *)buf = dev->regs.acreg;
                       	break;
                case SCR:
			*(uint16 *)buf = dev->regs.scr;
                       	break;
                case SSR:
			*(uint16 *)buf = dev->regs.ssr;
                       	break;
                case EBLR:
			*(uint16 *)buf = dev->regs.eblr;
                       	break;
                case MVR:
			*(uint16 *)buf = dev->regs.mvr;
                       	break;
		case SYSC:
			*(uint16 *)buf = dev->regs.sysc;
                       	break;
		case SYSS:
			*(uint16 *)buf = dev->regs.syss;
                       	break;
		case WER:
			*(uint16 *)buf = dev->regs.wer;
                       	break;
                case CFPS:
			*(uint16 *)buf = dev->regs.cfps;
                       	break;
                case RXFIFO_LVL:
			*(uint16 *)buf = dev->regs.rxfifo_lvl;
                       	break;
		case TXFIFO_LVL:
			*(uint16 *)buf = dev->regs.txfifo_lvl;
                       	break;
                case IER2:
			*(uint16 *)buf = dev->regs.ier2;
                       	break;
                case ISR2:
			*(uint16 *)buf = dev->regs.isr2;
                       	break;
                case MDR3:
			*(uint16 *)buf = dev->regs.mdr3;
                       	break;
                case TXDMA:
			*(uint16 *)buf = dev->regs.txdma;
                       	break;
		default:

			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}       

	return  No_exp;
}

static exception_t am335x_uart_write(conf_object_t* opaque, generic_address_t offset, void *buf,  size_t count)
{
	am335x_uart_dev *dev = opaque->obj;
	skyeye_uart_intf* skyeye_uart = dev->term;
	switch(offset)
	{
		case THR: 
			dev->regs.thr = *(uint16 *)buf;
			dev->regs.lsr &= (~(0x60));
			skyeye_uart->write(skyeye_uart->conf_obj, buf, count);
			dev->regs.lsr |= (0x60);
			break;
		case IER: /*IER, DLH, DLL*/
			if(IS_OP_MODE){
				if(READ_BIT(4, dev->regs.efr) == 1) /*IER[7:4] can only be written when EFR[4] = 1*/
					dev->regs.ier = *(uint16 *)buf;  /*operational mode ier write*/
				else
					dev->regs.ier |= (*(uint16 *)buf)&(~(0xF<< 4)); /*operational mode ier write*/ 
			}else if(IS_MODE_A){
					dev->regs.dlh = *(uint16 *)buf;  /*MODE A UART_DLH register write*/
			}else if(IS_MODE_B){
					dev->regs.dll = *(uint16 *)buf;  /*MODE B UART_DLL register write*/
			}
		case IIR: /*FCR, EFR*/
			if(IS_OP_MODE || IS_MODE_A){
				if(READ_BIT(4, dev->regs.efr) == 1) /* FCR[5:4] can only be written when EFR[4] = 1*/
					dev->regs.fcr = *(uint16 *)buf; /*operational mode fcr write*/
				else
					dev->regs.fcr |= (*(uint16 *)buf)&(~(3 << 4)); /*operational mode fcr write*/ 
			}else if(IS_MODE_B){
				dev->regs.efr = *(uint16 *)buf;  /*MODE B UART_EFR register write*/
			}
		       	break;
		case LCR:
			dev->regs.lcr = *(uint16 *)buf;
		       	break;
		case MCR:
			dev->regs.mcr = *(uint16 *)buf; 
                       	break;
		case LSR:
			dev->regs.lsr = *(uint16 *)buf;
                       	break;
                case MSR:/*MSR, TCR, XOFF1*/
			if(TCR_TLR_ENABLE){ /*Transmission control register (TCR) are accessible only when EFR[4] = 1 and MCR[6] = 1*/
				dev->regs.tcr = *(uint16 *)buf;   /*all mode UART_TCR write*/
			}else{
				if(IS_MODE_B){
					dev->regs.xoff1 = *(uint16 *)buf; /*MODE B UART_xoff1 write*/
				}
			}
			
                       	break;
		case SPR: /*SPR, TLR, XOFF2*/
			if(TCR_TLR_ENABLE){  /*trigger level register (TLR) are accessible only when EFR[4] = 1 and MCR[6] = 1*/
				dev->regs.tlr = *(uint16 *)buf;  /*all mode UART_TLR write*/
			}else{
				if(IS_OP_MODE | IS_MODE_A){
					dev->regs.spr = *(uint16 *)buf; /*operational mode UART_SPR write*/
				}else if(IS_MODE_B ){
					dev->regs.xoff2 = *(uint16 *)buf; /*MODE B UART_xoff2 write*/
				}
			}
                       	break;
                case MDR1:
			dev->regs.mdr1 = *(uint16 *)buf;
                       	break;
                case MDR2:
			dev->regs.mdr2 = *(uint16 *)buf;
                       	break;
                case SFLSR:
			dev->regs.sflsr = *(uint16 *)buf;
                       	break;
		case RESUME:
			dev->regs.resume = *(uint16 *)buf;
	               	break;
                case SFREGL:
			dev->regs.sfregl = *(uint16 *)buf;
                       	break;
                case SFREGH:
			dev->regs.sfregh = *(uint16 *)buf;
                       	break;
                case BLR:
			dev->regs.sfregh = *(uint16 *)buf;
                       	break;
                case ACREG:
			dev->regs.acreg = *(uint16 *)buf;
                       	break;
                case SCR:
			dev->regs.scr = *(uint16 *)buf;
                       	break;
                case SSR:
			dev->regs.ssr = *(uint16 *)buf;
                       	break;
                case EBLR:
			dev->regs.eblr = *(uint16 *)buf;
                       	break;
                case MVR:
			dev->regs.mvr = *(uint16 *)buf;
                       	break;
		case SYSC:
			dev->regs.sysc = *(uint16 *)buf;
			if(READ_BIT(1, dev->regs.sysc) == 1)
				SET_BIT(0, dev->regs.syss, 1);
                       	break;
		case WER:
			dev->regs.wer = *(uint16 *)buf;
                       	break;
                case CFPS:
			dev->regs.cfps = *(uint16 *)buf;
                       	break;
                case RXFIFO_LVL:
			dev->regs.rxfifo_lvl = *(uint16 *)buf;
                       	break;
		case TXFIFO_LVL:
			dev->regs.txfifo_lvl = *(uint16 *)buf;
                       	break;
                case IER2:
			dev->regs.ier2 = *(uint16 *)buf;
                       	break;
                case ISR2:
			dev->regs.isr2 = *(uint16 *)buf;
                       	break;
                case MDR3:
			dev->regs.mdr3 = *(uint16 *)buf;
                       	break;
                case TXDMA:
			dev->regs.txdma = *(uint16 *)buf;
                       	break;
		default:

			printf("read %s error offset %d : 0x%x\n",dev->obj->objname, offset, *(uint16*)buf);
			break;
	}
	return  No_exp;
}

static conf_object_t* new_am335x_uart(char* obj_name)
{
	am335x_uart_dev* am335x_uart = skyeye_mm_zero(sizeof(am335x_uart_dev));

	am335x_uart->obj = new_conf_object(obj_name, am335x_uart);

	memory_space_intf* io_memory = skyeye_mm_zero(sizeof(memory_space_intf));
	io_memory->conf_obj = am335x_uart->obj;
	io_memory->read = am335x_uart_read;
	io_memory->write = am335x_uart_write;

	SKY_register_interface((void*)io_memory, obj_name, MEMORY_SPACE_INTF_NAME);


	pthread_t pthread_id;
	//create_thread(hardware_trans, am335x_uart, &pthread_id);
	
	return am335x_uart->obj;
}

static exception_t reset_am335x_uart(conf_object_t* opaque, const char* args)
{
#if 1
	am335x_uart_dev *dev = opaque->obj;
	memset(&(dev->regs), 0, sizeof(dev->regs));
	dev->regs.iir = 0x1;      
	dev->regs.lsr = 0x60;      
	dev->regs.mdr1 = 0x7;     
        dev->regs.blr = 0x40;
        dev->regs.ssr = 0x40;
        dev->regs.wer  = 0xFF; 
        dev->regs.cfps = 0x69;
        dev->regs.freo_sel = 0x1A;
	dev->term = NULL;
        CreateFIFO(&dev->rx_fifo, 64);
        CreateFIFO(&dev->tx_fifo, 64);
#endif
	return No_exp;        
}


static exception_t free_am335x_uart(conf_object_t* conf_obj)
{

	return No_exp;
}

void init_am335x_uart(){
	static skyeye_class_t class_data = {
		.class_name = "am335x_uart",
		.class_desc = "am335x_uart",
		.new_instance = new_am335x_uart,
		.reset_instance = reset_am335x_uart,
		.free_instance = free_am335x_uart,
		.get_attr = NULL,
		.set_attr = am335x_uart_set_attr
	};

	SKY_register_class(class_data.class_name, &class_data);
}

