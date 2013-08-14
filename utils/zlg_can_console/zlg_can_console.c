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
* @file zlg_can_console.c
* @brief The console of zlg can
* @author Michael.Kang blackfin.kang@gmail.com
* @version 7849
* @date 2013-08-13
*/

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termio.h>
#include <unistd.h>

#ifndef __MINGW32__
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <windows.h>
#include <winsock2.h>
#endif
#ifndef __MINGW32__
#define TEST_CAN 1
#else
#define TEST_CAN 0
#endif
#if TEST_CAN
#else
#include "controlcan.h"
#endif
static int setup_zlg_can();
static int setup_netlink(char * hostp, char * portp);
static int can_transmit(char* buf , int nbytes);
static int can_receive(char* buf , int nbytes);
int test_zlg();

int main(int argc, char ** argv)
{
	int fd_socket;
	uint8_t buf[1024];
	char * hostp;
	char * portp;
#ifdef __MINGW32__
	if(argc == 1){ /* self test */
		char trans_buf[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
		test_zlg(trans_buf, 8);
		return 1;
	}
#endif
	
#ifndef __MINGW32__
	if (argc != 2) {
#else
	if (argc != 3) {
#endif
		printf("argc=%d, arg0=%s, arg1=%s\n", argc, argv[0], argv[1]);
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(1);
	}
#if 1
#ifndef __MINGW32__
	hostp = argv[0];
	portp = argv[1];
#else
	hostp = argv[1];
	portp = argv[2];
#endif
	//printf("hostp=%s, portp=%s\n", hostp, portp);
	int dev_status;
	dev_status = setup_zlg_can();
	fd_socket = setup_netlink(hostp, portp);

	#define MSG_LEN 8
	char msg_buf[MSG_LEN];

	do {
		int res;
		int count;
		int cmd = 0;
		recv(fd_socket, &cmd, sizeof(cmd), 0);
		//printf("In %s, cmd=0x%x\n", __FUNCTION__, cmd);
		int result = 1;
		send(fd_socket, &result, sizeof(result), 0);
		if(cmd & 0x800000){ /* write command */
			recv(fd_socket, msg_buf, MSG_LEN, 0);
			int i = 0;
			for(; i < 8; i++){
				//printf("In %s, recv[%d] = 0x%x\n", __FUNCTION__, i, msg_buf[i]);
			}
			if(dev_status < 0){
			}
			else /* send by real can device */
				can_transmit(msg_buf, MSG_LEN);
			send(fd_socket, &result, sizeof(result), 0);
		}
		else{ /* read command */
			if(dev_status < 0){
			}

			else /* receive from real can device */
				can_receive(msg_buf, MSG_LEN);
			send(fd_socket, msg_buf, MSG_LEN, 0);
			send(fd_socket, &result, sizeof(result), 0);
		}

	} while (1);
#endif
	return 0;
}
int setup_zlg_can()
{
	/* open, init and start can device*/
#if TEST_CAN
#else
	if(VCI_OpenDevice(VCI_USBCAN1,0,0)!=1)
	{
		printf("open real can deivce failed. use software can device\n");
		return -1;
	}
	VCI_INIT_CONFIG config;
	config.AccCode=0;
	config.AccMask=0xffffffff;
	config.Filter=1;
	//config.Mode=0;
	config.Mode = 0x2;
	config.Timing0=0;
	config.Timing1=0x14;
	
	if(VCI_InitCAN(VCI_USBCAN1,0,0,&config)!=1)
	{
		printf("init CAN error\n");
		goto ext;
	}

	if(VCI_StartCAN(VCI_USBCAN1,0,0)!=1)
	{
		printf("Start CAN error\n");
		goto ext;
	}
	return 0;
ext:	
	VCI_CloseDevice(VCI_USBCAN1,0);
#endif
	return -1;
}

#if 1
int setup_netlink(char * hostp, char * portp)
{
	int	port;
	int	num;
#ifndef __MINGW32__
	int	skt;

#else
	int err;
	SOCKET skt;
	WSADATA wsaData;
	/*
	 * initiates use of the Winsock DLL by a process
	 * shenoubang modified it 2012-12-4
	 */
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0) {
		/*
		* Tell the user that we could not find a usable
		* Winsock DLL.
		*/
		printf("WSAStartup failed with error: %d\n", err);
		return 1;
	}

#endif

	printf("Connecting to %s:%s\n", hostp, portp);
	if (sscanf(portp, "%d", &port)!=1 || port<1 || port>65535) {
		fprintf(stderr, "Connection port must be a number from 1 to 65535");
		exit(1);
	}
	struct	sockaddr_in target;
	target.sin_family = AF_INET;
#ifndef __MINGW32__
	target.sin_addr.s_addr = INADDR_ANY;
#else
	target.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
	target.sin_port = htons(port);

	for(num = 0; num < 3; num++) {

		skt = socket(AF_INET, SOCK_STREAM, 0);
		if (skt < 0) {
			perror("opening stream socket");
			exit(1);
		}

#ifndef __MINGW32__
		if (connect(skt, (struct sockaddr*)&target, sizeof(target)) != -1) goto gotit;
#else
		if(connect(skt, (SOCKADDR*)&target, sizeof(target)) != -1) goto gotit;
#endif
		printf("connecting error: %s\n", strerror(errno));
		fflush(stdout);
#ifndef __MINGW32__
		sleep(1);
#else
		Sleep(1);
#endif
		close(skt);
	}

	return -1;

gotit:
	fflush(stdout);
	;

	return skt;
}
int can_transmit(char* buf, int nbytes){
#if TEST_CAN
#else	
	//printf("In %s, begin transmit\n", __FUNCTION__);	
	VCI_CAN_OBJ send[3];
	send[0].ID=0;
	send[0].SendType=2;
	send[0].RemoteFlag=0;
	send[0].ExternFlag=1;
	send[0].DataLen=8;
	send[1]=send[0];
	send[2]=send[0];
	send[1].ID=1;
	send[2].ID=2;
	
	int i=0;
	int sendind=3;	
	for(i=0;i<send[0].DataLen;i++)
	{
		send[0].Data[i]=buf[i];
		send[1].Data[i]=buf[i];
		send[2].Data[i]=buf[i];
	}	
	int times=30;
	while(times--)
	{
		if(VCI_Transmit(VCI_USBCAN1,0,0,send,3)>0)
		{
			//printf("Send: %08X",send[0].ID);
			for(i=0;i<send[0].DataLen;i++)
			{
				//printf(" %08X",send[0].Data[i]);
			}
			//printf("\n");
			send[0].ID=sendind++;
			send[1].ID=sendind++;
			send[2].ID=sendind++;
		}
		else
			break;
		usleep(1);
	}
#endif
	return 0;
}
int can_receive(char* buf, int nbytes){
#if TEST_CAN
#else
	int reclen=0;
	VCI_CAN_OBJ rec[100];
	int i;
	{
		//printf("running....\n");
		if((reclen=VCI_Receive(VCI_USBCAN1,0,0,rec,100,100))>0)
		{
			//printf("IND:%d Receive: %08X",0,rec[reclen-1].ID);
			for(i=0;i<rec[reclen-1].DataLen;i++)
			{
				//printf(" %08X",rec[reclen-1].Data[i]);
				buf[i] = rec[reclen - 1].Data[i];
			}
			//printf("\n");
			
		}	
	}
#endif
	return 0;
}
#endif

#ifdef __MINGW32__
void receive_func(char* buf, int nbytes)  
{
	int reclen=0;
	VCI_CAN_OBJ rec[100];
	int i;
	{
		printf("running....\n");
		if((reclen=VCI_Receive(VCI_USBCAN1,0,0,rec,100,100))>0)
		{
			printf("IND:%d Receive: %08X",0,rec[reclen-1].ID);
			for(i=0;i<rec[reclen-1].DataLen;i++)
			{
				printf(" %08X",rec[reclen-1].Data[i]);
				buf[i] = rec[reclen - 1].Data[i];
			}
			printf("\n");
		}	
	}
	return;
}
void transmit_func(char* buf, int nbytes){
	VCI_CAN_OBJ send[3];
	send[0].ID=0;
	send[0].SendType=2;
	send[0].RemoteFlag=0;
	send[0].ExternFlag=1;
	send[0].DataLen=8;
	send[1]=send[0];
	send[2]=send[0];
	send[1].ID=1;
	send[2].ID=2;
	
	int i=0;
	
	for(i=0;i<send[0].DataLen;i++)
	{
		send[0].Data[i]= buf[i];
		send[1].Data[i]= buf[i];
		send[2].Data[i]= buf[i];
	}
	int ret;
	int times=3;
	int sendind=3;
	while(times--)
	{
		if(VCI_Transmit(VCI_USBCAN1,0,0,send,3)>0)
		{
			printf("Send: %08X",send[0].ID);
			for(i=0;i<send[0].DataLen;i++)
			{
				printf(" %08X",send[0].Data[i]);
			}
			printf("\n");
			send[0].ID=sendind++;
			send[1].ID=sendind++;
			send[2].ID=sendind++;
		}
		else
			break;
		usleep(1);
	}
	return;
}
int test_zlg(char* buf, int nbytes){
	setup_zlg_can();

	transmit_func(buf, nbytes);
	/* receive */
	char recv_buf[8];
	receive_func(recv_buf, 8);
	
	usleep(10);
	
ext:	
	VCI_CloseDevice(VCI_USBCAN1,0);
	return 0;
}
#endif
