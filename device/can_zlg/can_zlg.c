/* Copyright (C) 
* 2013 - Michael.Kang blackfin.kang@gmail.com
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
/**
* @file can_zlg.c
* @brief The interface for zlg USB CAN I device
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-06
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
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdint.h>

#include <stdlib.h>

#define DEBUG
#include <skyeye_log.h>

#include "skyeye_can_ops.h"
//#include "ControlCan.h"
#include "can_zlg.h"

#ifndef SKYEYE_BIN
const char* default_bin_dir = "/opt/skyeye/bin/";
#else
const char* default_bin_dir = SKYEYE_BIN;
#endif
const char* uart_prog = "zlg_can_console";

static int create_can(char * hostname, int port){
	pid_t pid;
	char port_str[32];
	char can_instance_prog[1024];
	//char * argv[]={"xterm","-e",uart_instance_prog,"localhost", "2345"};
	int bin_dir_len = strlen(default_bin_dir);
	memset(&can_instance_prog[0], '\0', 1024);
	strncpy(&can_instance_prog[0], default_bin_dir, bin_dir_len);
	strncpy(&can_instance_prog[bin_dir_len], uart_prog, strlen(uart_prog));

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
        	    	//printf("[child]connect to %s:%s!\n", hostname, port_str);
			int ret = execlp(can_instance_prog, hostname, port_str, (char *)NULL);
			perror("Child:");
			fprintf(stderr, "SKYEYE Error: run program %s failed!\n", can_instance_prog);

	        	_exit (-1);
        	}
		default:
			break;
	}
	return 0;
}

exception_t open_can_device(conf_object_t* obj){
	can_zlg_device *dev = obj->obj;
	exception_t ret;
	uint8_t buf[1024];
	#define	MAXHOSTNAME	256
	char	myhostname[MAXHOSTNAME];
	int sv_skt;
	struct hostent	* hp;
	int on, length;
	struct sockaddr_in server, from;
	char * froms;
	printf("In %s\n", __FUNCTION__);
	sv_skt = socket(AF_INET, SOCK_STREAM, 0);
	if (sv_skt < 0) SKYEYE_ERR("opening stream socket");

	/* enable the reuse of this socket if this process dies */
	if (setsockopt(sv_skt, SOL_SOCKET, SO_REUSEADDR, (uint8_t*)&on, sizeof(on))<0)
		SKYEYE_ERR("turning on REUSEADDR");

		/* bind it */
retry:
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(0);	/* bind to an OS selected local port */

	if (bind(sv_skt, (struct sockaddr *)&server, sizeof(server)) < 0) {
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
	if (getsockname(sv_skt, (struct sockaddr *) &server, &length)==-1)
		SKYEYE_ERR("getting socket name");

	if(listen(sv_skt, 1) < 0){
		perror("listen:");
	}

	gethostname(myhostname, MAXHOSTNAME);
	//printf("In %s, before main loop\n", __FUNCTION__);
	/* Create the client xterm */
	create_can(myhostname, ntohs(server.sin_port));
	/* main loop */
	struct	sockaddr_in client;
	client.sin_family = AF_INET;
	int len = sizeof(struct sockaddr);
	do {
		int socket_conn = accept(sv_skt, (struct sockaddr *)&client, &len);
		if(socket_conn > 0){
			dev->conn_socket = socket_conn;
			printf("get a client, socket_conn=%d\n", socket_conn);
			break;
		}
		else{
			printf("socket_conn=%d\n", socket_conn);
			perror("socket accept:");
		}
	} while (1);

	return ret;
}
exception_t close_can(){
	exception_t ret;
	//VCI_CloseDevice(nDeviceType, nDev)
	return ret;
}
exception_t stop_can(){
	exception_t ret;
	return ret;
}

exception_t start_can(conf_object_t* obj){
	exception_t ret;
	ret = open_can_device(obj);
	return ret;
}
exception_t can_transmit(conf_object_t* obj, void* addr, int nbytes){
	can_zlg_device *dev = obj->obj;
	int cmd = 0x800000;
	char* buf = addr;
	int result = 0;
	int n;
	printf("In %s, conn_socket=%d\n", __FUNCTION__, dev->conn_socket);
	n = send(dev->conn_socket, &cmd, sizeof(cmd), 0);
	printf("the ret of send is %d\n", n);
	recv(dev->conn_socket, &result, sizeof(result), 0);
	printf("In %s, result=%d\n", __FUNCTION__, result);
	send(dev->conn_socket, buf, nbytes, 0);
	recv(dev->conn_socket, &result, sizeof(result), 0);
	printf("In %s, after send msg, result=%d\n", __FUNCTION__, result);
	exception_t ret;
	printf("In %s\n", __FUNCTION__);
	#if 0
	VCI_CAN_OBJ vco;
	memset(&vco, '\0', sizeof(VCI_CAN_OBJ));
	vco.ID = 0x00000000;
	vco.SendType = 0;
	vco.RemoteFlag = 0;
	vco.ExternFlag = 0;
	vco.DataLen = 8;
	lReg = VCI_Transmit(nDeviceType, nDeviceInd, nCANInd, &vco, i);
	#endif
	return ret;
}

exception_t can_receive(conf_object_t* obj, void* addr, int nbytes){
	can_zlg_device *dev = obj->obj;
	int cmd = 0x000000;
	char* buf = addr;
	int result = 0;
	int n;
	printf("In %s, conn_socket=%d\n", __FUNCTION__, dev->conn_socket);
	n = send(dev->conn_socket, &cmd, sizeof(cmd), 0);
	printf("the ret of send is %d\n", n);
	recv(dev->conn_socket, &result, sizeof(result), 0);
	printf("In %s, result=%d\n", __FUNCTION__, result);
	recv(dev->conn_socket, buf, nbytes, 0);
	recv(dev->conn_socket, &result, sizeof(result), 0);
	int i = 0;
	for(; i < 8; i++)
		printf("In %s, buf[%d] = 0x%x\n", __FUNCTION__, i, buf[i]);


	exception_t ret;
	printf("In %s\n", __FUNCTION__);
	#if 0
	VCI_CAN_OBJ vco[100];
	lRet = VCI_Receive(nDeviceType, nDeviceInd, nCANInd, vco, 100, 400);
	#endif
	return ret;
}
static conf_object_t* new_can_zlg(char* obj_name){
	can_zlg_device* dev = skyeye_mm_zero(sizeof(can_zlg_device));
	dev_info_t* info =  skyeye_mm_zero(sizeof(dev_info_t));
	dev->obj = new_conf_object(obj_name, dev);
	dev->info = info;

	can_ops_intf* ops = skyeye_mm_zero(sizeof(can_ops_intf));
	ops->obj = dev->obj;
	ops->start = start_can;
	ops->stop = stop_can;
	ops->transmit = can_transmit;
	ops->receive = can_receive;
	dev->ops = ops;
	SKY_register_interface(ops, obj_name, CAN_OPS_INTF_NAME);	
	printf("In %s, ops=0x%x\n", __FUNCTION__, ops);
	start_can(dev->obj);

	info->device_type = 20;
	info->device_id = 0;
	info->reserved = 9600; /* baud rate */
	return dev->obj;
}
void free_can_zlg(conf_object_t* dev){
	
}

void init_can_zlg(){
	static skyeye_class_t class_data = {
		.class_name = "can_zlg",
		.class_desc = "CAN USB I device of ZLG",
		.new_instance = new_can_zlg,
		.free_instance = free_can_zlg,
		.get_attr = NULL,
		.set_attr = NULL
	};
		
	SKY_register_class(class_data.class_name, &class_data);
}
