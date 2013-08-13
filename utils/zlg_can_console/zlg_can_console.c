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


static int setup_zlg_can();
static int setup_netlink(char * hostp, char * portp);
static int can_transmit(char* buf , int nbytes);
static int can_receive(char* buf , int nbytes);

int main(int argc, char ** argv)
{
	int fd_socket;
	uint8_t buf[1024];
	char * hostp;
	char * portp;
	
	if (argc != 2) {
		printf("argc=%d, arg0=%s, arg1=%s\n", argc, argv[0], argv[1]);
		fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
		exit(1);
	}

	hostp = argv[0];
	portp = argv[1];
	printf("hostp=%s, portp=%s\n", hostp, portp);
	setup_zlg_can();
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
			can_transmit(msg_buf, MSG_LEN);
			send(fd_socket, &result, sizeof(result), 0);
		}
		else{ /* read command */
			can_receive(msg_buf, MSG_LEN);
			send(fd_socket, msg_buf, MSG_LEN, 0);
			send(fd_socket, &result, sizeof(result), 0);
		}

	} while (1);
}

int setup_zlg_can()
{
	/* open, init and start can device*/
	return 0;
}

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
	return 0;
}
int can_receive(char* buf, int nbytes){
	return 0;
}