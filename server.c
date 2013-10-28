/**********************************
server.c: the source file of the server in UDP transmission 
***********************************/
#include "headsock.h"
                                    
// Transmitting and receiving function									
void receive_data(int sockfd);		

int main(void)
{
	int sockfd;
	struct sockaddr_in ser_addr;

	// Create a socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)		
	{
		printf("error in socket!");
		exit(1);
	}
	
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(MYUDP_PORT);
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&(ser_addr.sin_zero),8);
	 
	// Bind the socket
	if (bind(sockfd, (struct sockaddr *) &ser_addr, sizeof(struct sockaddr)) == -1)		
	{
		printf("Error in binding.\n");
		exit(1);
	}
	
	printf("Start receiving data.\n");
	
	// Receive packet and response  
	receive_data(sockfd);		                             
	close(sockfd);
	exit(0);
}

void receive_data(int sockfd)
{
	char buffer[BUFSIZE], received_data[DATALEN];
	FILE *file_pointer;
	struct ack_so ack;
	struct sockaddr_in client_addr;
	int addrlen, n, end_of_transmission = 0;
	long lseek = 0;
	
	addrlen = sizeof (struct sockaddr_in);
	int c = 0;
	
	while (!end_of_transmission)
	{
		printf("c = %d\n",c);
		c++;
		
		// Receive the packet from client
		n = recvfrom(sockfd, &received_data, DATALEN, 0, (struct sockaddr *)&client_addr, &addrlen);
		if (n == -1)		
		{
			printf("Error when receiving.\n");
			exit(1);
		}
		
		// If received a packet with error
		else if (n == 0)			
		{
			printf("Received a packet with error.\n");
			ack.num = NACK_CODE;		
		}
		
		// If received a packet without error
		else						
		{
			// If it is the end of the file
			if (received_data[n-1] == '\0')			
			{
				end_of_transmission = 1;
				n --;
			}
			memcpy((buffer+lseek), received_data, n);
			lseek += n;
			ack.num = ACK_CODE;
		}
		ack.len = 0;
	
		// Send the ACK or NACK to server
		if ((n = sendto(sockfd, &ack, 2, 0, (struct sockaddr *)&client_addr, addrlen)) == -1)			
		{
			printf("Error when sending ACK/NACK.\n");								
			exit(1);
		} 
	}                               

	if ((file_pointer = fopen ("output_file.txt","wt")) == NULL)
	{
		printf("File doesn't exit\n");
		exit(0);
	}
		
	// Write the data into file
	fwrite (buffer, 1, lseek, file_pointer);			
	fclose(file_pointer);
	printf("A file has been successfully received!\nThe total data received is %d bytes.\n", (int)lseek);
}