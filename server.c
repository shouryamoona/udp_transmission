/**********************************
server.c: the source file of the server in udp transmission for a large packet
***********************************/


#include "headsock.h"

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
	
	while (1)
	{
		printf("waiting for data\n");
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
	char recvs[DATALEN];
	struct ack_so ack;
	int end, n = 0;
	long lseek=0;
	end = 0;
	
	printf("receiving data!\n");
	
	struct sockaddr_in their_addr;
	int sin_size = sizeof (struct sockaddr_in);
	
	while(!end)
	{
		if ((n= recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&their_addr, &sin_size))==-1)                                   //receive the packet
		{
			printf("error when receiving\n");
			exit(1);
		}
		if (recvs[n-1] == '\0')									//if it is the end of the file
		{
			end = 1;
			n --;
		}
		memcpy((buf+lseek), recvs, n);
		lseek += n;
	}
	
	ack.num = 1;
	ack.len = 0;
	
	
	// (n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&their_addr, sin_size))==-1
	
	if ((n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&their_addr, sin_size))==-1)
	{
			printf("send error!");								//send the ack
			exit(1);
	}                                                 

	if ((fp = fopen ("myTCPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	
	fwrite (buf , 1 , lseek , fp);								//write the data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}
