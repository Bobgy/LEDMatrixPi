#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int led_digit[10][8]={
	0x00,0x00,0x3E,0x41,0x41,0x3E,0x00,0x00, //0
	0x00,0x00,0x21,0x7F,0x01,0x00,0x00,0x00, //1
	0x00,0x00,0x23,0x45,0x49,0x31,0x00,0x00, //2
	0x00,0x00,0x22,0x49,0x49,0x36,0x00,0x00, //3
	0x00,0x00,0x0E,0x32,0x7F,0x02,0x00,0x00, //4
	0x00,0x00,0x79,0x49,0x49,0x46,0x00,0x00, //5
	0x00,0x00,0x3E,0x49,0x49,0x26,0x00,0x00, //6
	0x00,0x00,0x60,0x47,0x48,0x70,0x00,0x00, //7
	0x00,0x00,0x36,0x49,0x49,0x36,0x00,0x00, //8
	0x00,0x00,0x32,0x49,0x49,0x3E,0x00,0x00  //9
};

int row[8]={7,6,5,4,3,2,1,0}; //abc
int lin[8]={8,9,10,11,12,13,14,15}; //123
void shownum(int num){
	for(int j=0;j<8;j++){
		for(int k=0;k<8;k++)
			digitalWrite(lin[k],HIGH);
		digitalWrite(lin[j],LOW);
		for(int i=0;i<8;i++){
			if((led_digit[num][j]>>i)&1){
				digitalWrite(row[i],HIGH);
			}
			else
				digitalWrite(row[i],LOW);
			delay(0);
		}
	}
}

int main (int argc, char* argv[])
{
	int cfd;
	int recbytes;
	int sin_size;
	char buffer[1024]={0};
	struct sockaddr_in s_add,c_add;
	unsigned short portnum=0x8888;
	printf("Hello,welcome to client !\r\n");

#ifdef _SOCKET
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return -1;
	}
	printf("socket ok !\r\n");

	bzero(&s_add,sizeof(struct sockaddr_in));
	s_add.sin_family=AF_INET;
	s_add.sin_addr.s_addr= inet_addr("192.168.9.101");
	s_add.sin_port=htons(portnum);
	printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

	if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return -1;
	}
	printf("connect ok !\r\n");

	if(-1 == (recbytes = read(cfd,buffer,1024)))
	{
		printf("read data fail !\r\n");
		return -1;
	}
	printf("read ok\r\nREC:\r\n");
	buffer[recbytes]='\0';
	int number=buffer[0]-48;
	printf("%d\r\n",number);
	close(cfd);
#else
	int number = 4;
	if(argc>1){
		sscanf(argv[1], "%d", &number);
	}
#endif
	printf("number is %d\n", number);
	int flag=1;
	if (wiringPiSetup () == -1)
		exit (1) ;
	for(int i=0;i<8;i++){
		pinMode (row[i], OUTPUT);
		pinMode (lin[i], OUTPUT);
	}
	for(;;){
		shownum(number%10);
	}
	return 0 ;
}
