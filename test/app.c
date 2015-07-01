#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define _SOCKET

char buffer[100];

int main (int argc, char* argv[])
{
	int cfd;
	int sin_size;
	struct sockaddr_in s_add,c_add;
	unsigned short portnum=0x8888;
	printf("Hello,welcome to client !\r\n");

	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return -1;
	}
	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr("192.168.9.1");
	s_add.sin_port=htons(portnum);
	printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return -1;
	}
	printf("connect ok !\r\n");
	if (argc > 1) {
		strcpy(buffer, argv[1]);
	} else scanf("%s", buffer);
	printf("sending %s\r\n", buffer);
	int ret = send(cfd, buffer, strlen(buffer)+1, 0);
	if (ret < 0) {
		printf("write fail with ECODE: %d!\r\n", -ret);
		close(cfd);
		return -1;
	}
	printf("\r\nwrite ok!\r\n");
	ret = close(cfd);
	if (ret < 0) {
		printf("send fail with ECODE: %d!\r\n", -ret);
		return -1;
	}

	return 0 ;
}
