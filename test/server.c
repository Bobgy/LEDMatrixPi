#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

char buffer[200];

int main(int agrc, char* argv)
{
    int sfp, nfp;
    struct sockaddr_in s_add,c_add;
    int sin_size;
    unsigned short portnum=0x8888;
    printf("Hello,welcome to my server !\r\n");
    sfp = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == sfp)
    {
        printf("socket fail ! \r\n");
        return -1;
    }
    printf("socket ok !\r\n");

    bzero(&s_add,sizeof(struct sockaddr_in));
    s_add.sin_family=AF_INET;
    s_add.sin_addr.s_addr=htonl(INADDR_ANY);
    s_add.sin_port=htons(portnum);

    if(-1 == bind(sfp,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
    {
        printf("bind fail !\r\n");
        return -1;
    }
    printf("bind ok !\r\n");

    if(-1 == listen(sfp,5))
    {
        printf("listen fail !\r\n");
        return -1;
    }
    printf("listen ok\r\n");
    while(1)
    {
        sin_size = sizeof(struct sockaddr_in);

        nfp = accept(sfp, (struct sockaddr *)(&c_add), &sin_size);
        if(-1 == nfp)
        {
            printf("accept fail !\r\n");
            return -1;
        }
        printf("accept ok!\r\nServer start get connect from %#x : %#x\r\n",ntohl(c_add.sin_addr.s_addr),ntohs(c_add.sin_port));

        int len = 0;
        if(-1 == (len = recv(nfp, buffer, 128, 0)))
    	{
    		printf("read data fail !\r\n");
    		return -1;
    	}
    	printf("read ok\r\n");
        printf("len = %d\r\n", len);
    	buffer[len]='\0';
        puts(buffer);
    	close(nfp);

        FILE *fp = fopen("/dev/ledmatrix", "w");
    	if (fp == NULL) {
    		puts("Cannot open /dev/ledmatrix");
    		return -1;
    	}
        len = strlen(buffer);
    	for (int i=0; i < len; i++) {
    		fputc(buffer[i], fp);
    		fflush(fp);
    		sleep(1);
    	}
    	fclose(fp);
    }
    close(sfp);
    return 0;
}
