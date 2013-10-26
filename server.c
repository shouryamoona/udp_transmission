/**********************************
server.c: the source file of the server in udp transmission for a large packet
***********************************/


#include "headsock.h"

#define BACKLOG 10

//void str_ser(int sockfd, struct sockaddr *addr, int addrlen);                                                        // transmitting and receiving function
void str_ser(int sockfd);

int main(void)
{
	int sockfd;
	struct sockaddr_in my_addr;
	//struct sockaddr_in their_addr;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )			//create socket
	{
		printf("error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYUDP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(my_addr.sin_zero), 8);
	  
	if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) == -1 )				//bind socket
	{
		printf("error in binding");
		exit(1);
	}
	
	
	printf("receiving start\n");
	while (1)
	{
		str_ser(sockfd);
		//str_ser(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));                                          //receive packet and response
	}
	close(sockfd);
	exit(0);
}
//**************************
//void str_ser(int sockfd, struct sockaddr *addr, int addrlen)
void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	struct pack_so recvs;
	struct ack_so ack;
	int end, n = 0, ci, lsize=1;
	
	ci = end = ack.num = 0;
	
	
	struct sockaddr_in addr;

	int addrlen = sizeof (struct sockaddr_in);

	while(ci < lsize)
	{
		if ((n= recvfrom(sockfd, &recvs, MAXSIZE, 0, (struct sockaddr *)&addr, &addrlen))==-1)                                   //receive the packet
		{
			printf("receiving error!\n");
			return;
		}
		else printf("%d data received\n", n);
		if (ci == 0) {
			lsize = recvs.len;								//copy the data length
			memcpy(buf, recvs.data, (n-HEADLEN));			//copy the data
			ci += n-HEADLEN;
		}
		else {
			memcpy((buf+ci), &recvs, n);
			ci += n;
		}
	}
	ack.len = 0;
	ack.num = 1;
//	memcpy(buf, recvs.data, recvs.len);
	sendto(sockfd, &ack, 2, 0,(struct sockaddr*) &addr, sizeof addr);
//	send(sockfd, &ack, 2, 0);                                                  //send ACK or NACK

	if((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	printf("the data received: %d\n", ci);
	printf("the file size received: %d\n", lsize);
	fwrite (buf , 1 , lsize, fp);								//write the data into file
	fclose(fp);
	printf("a file has been successfully received!\n");
}
