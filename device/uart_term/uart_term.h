#ifndef __SKYEYE_UART_TERM_H__
#define __SKYEYE_UART_TERM_H__

#include "skyeye_log.h"
#include "skyeye_uart_ops.h"

#ifndef __MINGW32__
typedef char BOOL;
#endif

typedef struct receive_t{
	uint8_t* rec_buf;
	int	rec_head;
	int	rec_tail;
	int	rec_count;
}rec_t;

typedef struct skyeye_uart_term{
	conf_object_t* obj;
	rec_t*	receive;
	char*	obj_name;
	int	mod;
#ifndef __MINGW32__
	int	socket;
#else
	SOCKET socket;
#endif
	BOOL	attached;
}uart_term_device;

#define MAX_REC_NUM	1024
#define DBG_UART	1
#define DEBUG_UART(fmt, ...)      if(DBG_UART){							\
					skyeye_log(Debug_log, __func__, fmt, ## __VA_ARGS__);	\
				}
#endif
