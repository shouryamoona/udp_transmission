/***********************************
client.c: the source file of the client in UDP transmission
***********************************/
#include "headsock.h"

// Packet transmission function
float transmit_data(FILE *file_pointer, int sockfd,struct sockaddr *ser_addr, long *data_sent);			

// Calculate the time interval between out and in
void calculate_time_elapsed(struct timeval *out, struct timeval *in);			

int main(int argc, char **argv)
{
	int sockfd;
	float time_elapsed, transmission_rate;
	long data_sent;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *file_pointer;

	if (argc != 2) 
	{
		printf("Parameters do not match.\n");
		exit(0);
	}

	// Get host's information
	if ((sh=gethostbyname(argv[1])) == NULL)			
	{             				
		printf("Error when calling gethostbyname.\n");
		exit(0);
	}

	// Create the socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)		  
	{
		printf("Error when creating socket.\n");
		exit(0);
	}
	
	addrs = (struct in_addr **)sh->h_addr_list;
	// Print server's information
	printf("canonical name: %s\n", sh->h_name);			
	
	for (pptr = sh->h_aliases; *pptr != NULL; pptr++)
		printf("The aliases name is: %s\n", *pptr);
	
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
			break;
		default:
			printf("Unknown address type.\n");
			break;
	}
	
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero),8);

	// Open local file to read the data
	if((file_pointer = fopen("myfile.txt","r+t")) == NULL)		  
	{
		printf("File does not exit.\n");
		exit(0);
	}
	
	// Perform the transmission and receiving
	time_elapsed = transmit_data(file_pointer, sockfd, (struct sockaddr *)&ser_addr, &data_sent);			
	
	if (time_elapsed != -1)
	{
		// Calculate the average transmission rate
		transmission_rate = (data_sent/(float)time_elapsed);			
		printf("Ave Time(ms) : %.3f, Ave Data sent(byte): %d\nAve Data rate: %f (Kbytes/s).\n", time_elapsed, (int)data_sent, transmission_rate);
	}

	close(sockfd);
	fclose(file_pointer);
	exit(1);
}

float transmit_data(FILE *file_pointer, int sockfd, struct sockaddr *ser_addr, long *data_sent)
{
	char *buffer, send_data[DATALEN];
	long file_size,total_bytes_sent;
	struct ack_so ack;
	int n, packet_length, addrlen;
	float time_elapsed = 0.0;
	double random_probability = 0.0;
	struct timeval send_time, receive_time;
	struct sockaddr_in client_addr;
	
	addrlen = sizeof (struct sockaddr_in);
	total_bytes_sent = 0;
	
	fseek (file_pointer ,0 , SEEK_END);
	file_size = ftell (file_pointer);
	rewind (file_pointer);
	
	printf("The file length is %d bytes.\n", (int)file_size);
	printf("The packet length is %d bytes.\n",DATALEN);
	
	// Allocate memory to contain the whole file
	buffer = (char *) malloc (file_size);			 
	if (buffer == NULL) 
		exit (0);
	
	// Copy the file into the buffer
	fread (buffer,1,file_size,file_pointer);			  					
	
	// Append the null character at the end of the buffer
	buffer[file_size] ='\0';		
	
	gettimeofday(&send_time,NULL);			
	int c = 0;
	srand(time(NULL));
	
	while (total_bytes_sent <= file_size)
	{
		printf("c = %d\n",c);
		c++;
		if ((file_size + 1 - total_bytes_sent) <= DATALEN)
			packet_length = file_size + 1 - total_bytes_sent;
		else 
			packet_length = DATALEN;
			
		memcpy(send_data,(buffer + total_bytes_sent), packet_length);
		
		// Calculate random probability value to send a packet with or without error
		random_probability = (double)rand()/(double)RAND_MAX;
		if (random_probability > ERROR_PROBABILITY)			
		{
			// Send a packet without error
			n = sendto(sockfd, &send_data, packet_length, 0, ser_addr, sizeof (struct sockaddr_in));
			if (n == -1) 
			{
				printf("Error sending data packet.\n");								
				exit(0);
			}
		}
		else			
		{
			// Send a packet with error
			n = sendto(sockfd, &send_data, BAD_PACKET_LENGTH, 0, ser_addr, sizeof (struct sockaddr_in));
			if (n == -1) 
			{
				printf("Error sending data packet.\n");								
				exit(0);
			}
		}
		
		// Receive ACK or NACK from client
		if ((n = recvfrom(sockfd, &ack, 2, 0, (struct sockaddr *)&client_addr, &addrlen)) == -1) 		
		{	        
			printf("Error receiving ACK or NACK.\n");
			exit(0);
		}
	
		// If received NACK, continue and retransmit last packet.
		if ((ack.num == NACK_CODE) || (ack.len != 0))         		
		{
			printf("Error in transmission. Retransmitting packet.\n");
			continue;
		}	
		total_bytes_sent += packet_length;
	}
		
	gettimeofday(&receive_time, NULL);
	*data_sent = total_bytes_sent;                                                         
	calculate_time_elapsed(&receive_time, &send_time);				
	// Get the whole transmission time
	time_elapsed += (receive_time.tv_sec)*1000.0 + (receive_time.tv_usec)/1000.0;
	return(time_elapsed);	
}

void calculate_time_elapsed(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) < 0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}