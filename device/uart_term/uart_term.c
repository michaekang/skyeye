/*
 * Copyright (C)
 * 2011 - Michael.Kang blackfin.kang@gmail.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

/*
 * @file uart_term.c
 * @brief The implementation of skyeye uart term
 * @author David.Yu keweihk@gmail.com
 * @version 78.77
 * @date 2012-2-2
 */

#include <skyeye_types.h>
#include <skyeye_sched.h>
#include <skyeye_signal.h>
#include <skyeye_class.h>
#include <skyeye_interface.h>
#include <skyeye_obj.h>
#include <skyeye_mm.h>
#include <memory_space.h>
#include <skyeye_device.h>
#include "skyeye_thread.h"
/* linux head file */
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdint.h>


#include "uart_term.h"

#ifndef SKYEYE_BIN
const char* default_bin_dir = "/opt/skyeye/bin/";
#else
const char* default_bin_dir = SKYEYE_BIN;
#endif
const char* uart_prog = "uart_instance";

static exception_t uart_term_raise(conf_object_t* object, int line_no)
{
	/* Get vic interrupt interface */
	general_signal_intf* vic_signal = (general_signal_intf*)SKY_get_interface(object, GENERAL_SIGNAL_INTF_NAME);
	vic_signal->raise_signal(vic_signal->conf_obj, line_no);

	return 0;
}

static exception_t uart_term_down(conf_object_t* object, int line_no)
{
	return 0;
}

static exception_t uart_term_read(conf_object_t *opaque, void* buf, size_t count)
{
	uart_term_device *dev = opaque->obj;
	rec_t* receive = dev->receive;
	int i = 0;
	char* rec_buf = buf;
	if (dev->attached) {
		/* move the charaters in buffer */
		while(i < count && (receive->rec_tail < receive->rec_count)){
			if(receive->rec_head >= receive->rec_tail){
				/* No data */
				receive->rec_head = 0;
				receive->rec_tail = 0;
				return Not_found_exp;
			}
			else{
				rec_buf[i++] = receive->rec_buf[receive->rec_head++];
				if(receive->rec_head == receive->rec_tail){
					/* data have been read*/
					receive->rec_head = 0;
					receive->rec_tail = 0;
				}

			}
		}
		DEBUG_UART("rec_buf %s\n", rec_buf);
        }

	return No_exp;
}

static exception_t uart_term_write(conf_object_t *opaque, void* buf, size_t count)
{
	int ret = -1;
	uart_term_device *dev = opaque->obj;
	if(dev->attached){
		ret = write(dev->socket, buf, count);
		if(ret < 0)
			return Invarg_exp;
	}
	return No_exp;
}
#define DBG_XTERM
static int create_term(uart_term_device* dev_uart, int port){
	pid_t pid;
	char port_str[32];
	char uart_instance_prog[1024];
	int bin_dir_len = strlen(default_bin_dir);
	memset(&uart_instance_prog[0], '\0', 1024);
	strncpy(&uart_instance_prog[0], default_bin_dir, bin_dir_len);
	strncpy(&uart_instance_prog[bin_dir_len], uart_prog, strlen(uart_prog));

	sprintf(port_str, "%d", port);
	switch (pid = fork())
	{
		case -1:
		{
			perror("The fork failed!");
			break;
		}
		case 0:
		{
#ifdef DBG_XTERM
			int ret = execlp("xterm","xterm", "-hold", "-e", uart_instance_prog, dev_uart->obj_name, port_str, (char *)NULL);
#else
			int ret = execlp("xterm","xterm", "-e", uart_instance_prog, dev_uart->obj_name, port_str, (char *)NULL);
#endif
			perror("Child:");
			fprintf(stderr, "SKYEYE Error:We need xterm to run the console of uart.Please check if you installed it correctly.\n");

			_exit (-1);
		}
		default:
			break;
	}
	return 0;
}

static int create_uart_console(void* uart){
	uart_term_device* dev_uart = ((conf_object_t*)uart)->obj;
	rec_t* receive = dev_uart->receive;
	struct hostent	* hp;
	int on, length;
	struct sockaddr_in server, from;
	char * froms;
	printf("In %s\n", __FUNCTION__);
	int term_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (term_socket < 0) SKYEYE_ERR("opening stream socket");

	/* enable the reuse of this socket if this process dies */
	if (setsockopt(term_socket, SOL_SOCKET, SO_REUSEADDR, (uint8_t*)&on, sizeof(on))<0)
		SKYEYE_ERR("turning on REUSEADDR");
retry:
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(0);	/* bind to an OS selected local port */

	if (bind(term_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		switch (errno) {
		case EAGAIN:
			goto retry;

		case EADDRINUSE:
			SKYEYE_ERR("Port is already in use\n");
		default:
			SKYEYE_ERR("binding tcp stream socket");
		}
	}

	length = sizeof(server);
	if (getsockname(term_socket, (struct sockaddr *) &server, &length) == -1)
		SKYEYE_ERR("getting socket name");

	listen(term_socket, 1);
	/* Create the client xterm */
	create_term(dev_uart, ntohs(server.sin_port));
	/* main loop */
	do {
		if (!dev_uart->attached) {
			printf("Waiting for connection to %s:%d", dev_uart->obj_name, ntohs(server.sin_port));
			length = sizeof(from);
			/* save the uart socket */
			dev_uart->socket = accept(term_socket, (struct sockaddr *)&from, (int*)&length);

			hp = gethostbyaddr((char *)&from.sin_addr, 4, AF_INET);
			if (hp == (struct hostent *)0) {
				froms = inet_ntoa(from.sin_addr);
				fprintf(stderr,"cant resolve hostname for %s\n", froms);
			} else {
				froms = hp->h_name;
			}
			dev_uart->attached = 1;
		}
		else { /* begin receive data. */
			int res;
			struct pollfd fds;

#define	POLL_TIMEOUT	-1		/* wait forever ? FIXME ? */
			fds.fd = dev_uart->socket;
			fds.events = POLLIN|POLLPRI;
#if HOST_OS_SOLARIS9		/* { */
			fds.events |= POLLRDNORM|POLLRDBAND;
#endif				/* } */
			fds.revents = 0;

			res = poll(&fds, 1, POLL_TIMEOUT);

			if (fds.revents & POLLIN) {
				if(receive->rec_tail >= receive->rec_count)
					fprintf(stderr, "Overflow for uart link.\n");
				else
					res = read(dev_uart->socket, receive->rec_buf + receive->rec_tail, receive->rec_count - receive->rec_tail);
				if (res == 0) {
					/* a read of 0 bytes is an EOF */
					dev_uart->attached = 0;
					close(dev_uart->socket);
				} else if (res<0) {
					perror("read");
				} else {
					/* expand receive buf */
					receive->rec_tail += res;
					DEBUG_UART("recieve %s\n", receive->rec_buf);
				}
			}
		}//if (fds.revents & POLLIN)
	} while (1);

	return 0;
}



static conf_object_t* new_uart_term(char* obj_name){
	DEBUG_UART("In %s New uart term %s\n", __FILE__, obj_name);
	uart_term_device* dev_uart = skyeye_mm_zero(sizeof(uart_term_device));
	rec_t* receive = skyeye_mm_zero(sizeof(rec_t));
	dev_uart->obj = new_conf_object(obj_name, dev_uart);
	dev_uart->obj_name = obj_name;

	/* initial recevie */
	dev_uart->receive = receive;
	receive->rec_buf = skyeye_mm_zero(MAX_REC_NUM);
	receive->rec_head = 0;
	receive->rec_tail = 0;
	receive->rec_count = MAX_REC_NUM;

	/* Register io function to the object */
	skyeye_uart_intf* uart_method = skyeye_mm_zero(sizeof(skyeye_uart_intf));
	uart_method->conf_obj = dev_uart->obj;
	uart_method->read = uart_term_read;
	uart_method->write = uart_term_write;
	SKY_register_interface(uart_method, obj_name, SKYEYE_UART_INTF);

	dev_uart->attached = 0;
	dev_uart->socket = -1;
	dev_uart->mod= 3;

	pthread_t pthread_id;
	create_thread(create_uart_console, (void*)dev_uart->obj, &pthread_id);

	return dev_uart->obj;
}

void free_uart_term(conf_object_t* dev){

}

void init_uart_term(){
	static skyeye_class_t class_data = {
		.class_name = "uart_term",
		.class_desc = "uart term",
		.new_instance = new_uart_term,
		.free_instance = free_uart_term,
		.get_attr = NULL,
		.set_attr = NULL
	};

	SKY_register_class(class_data.class_name, &class_data);
}
