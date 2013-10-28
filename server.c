/**********************************
server.c: the source file of the server in UDP transmission 
***********************************/
#include "headsock.h"
                                                      
void str_ser(int sockfd);		// transmitting and receiving function

int main(void)
{
	int sockfd;
	struct sockaddr_in ser_addr;

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )		//create socket
	{
		printf("error in socket!");
		exit(1);
	}
	
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(ser_addr.sin_zero), 8);
	  
	if (bind(sockfd, (struct sockaddr *) &ser_addr, sizeof(struct sockaddr)) == -1 )		//bind socket
	{
		printf("error in binding");
		exit(1);
	}
	
	printf("waiting for data\n");
	str_ser(sockfd);		//receive packet and response                               
	close(sockfd);
	exit(0);
}

void str_ser(int sockfd)
{
	char buf[BUFSIZE];
	FILE *fp;
	char recvs[DATALEN];
	struct ack_so ack;
	struct sockaddr_in client_addr;
	int end, addrlen, n = 0;
	long lseek=0;
	
	addrlen = sizeof (struct sockaddr_in);
	end = 0;
	int c = 0;
	printf("receiving data!\n");
	
	while(!end)
	{
		printf("c = %d\n",c);
		c++;
		n= recvfrom(sockfd, &recvs, DATALEN, 0, (struct sockaddr *)&client_addr, &addrlen);
		if (n == -1)		//receive the packet
		{
			printf("Error when receiving.\n");
			exit(1);
		}
		
		else if (n == 0)			// if received a bad frame
		{
			ack.num = NACK_CODE;		
			printf("Received a bad frame\n");
		}
		
		else						// if received a good frame
		{
			if (recvs[n-1] == '\0')			//if it is the end of the file
			{
				end = 1;
				n --;
			}
			memcpy((buf+lseek), recvs, n);
			lseek += n;
			ack.num = ACK_CODE;
		}
		
		ack.len = 0;
	
		if ((n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&client_addr, addrlen)) == -1 )			//send the ack or nack
		{
			printf("send error!");								
			exit(1);
		} 
	}                               

	if ((fp = fopen ("myUDPreceive.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
	
	fwrite (buf , 1 , lseek , fp);			//write the data into file
	fclose(fp);
	printf("a file has been successfully received!\nthe total data received is %d bytes\n", (int)lseek);
}