/***********************************
client.c: the source file of the client in UDP transmission
***********************************/

#include "headsock.h"

// Packet transmission function
float str_cli(FILE *fp, int sockfd,struct sockaddr *ser_addr, long *len);			

// Calculate the time interval between out and in
void tv_sub(struct  timeval *out, struct timeval *in);			

int main(int argc, char **argv)
{
	int sockfd;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) 
		printf("Parameters do not match.\n");

	// Get host's information
	if ((sh=gethostbyname(argv[1])) == NULL)			
	{             				
		printf("Error when calling gethostbyname.\n");
		exit(0);
	}

	// Create the socket
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1 )		  
	{
		printf("error in socket");
		exit(1);
	}
	
	addrs = (struct in_addr **)sh->h_addr_list;
	// Print server's information
	printf("canonical name: %s\n", sh->h_name);			
	
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	
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
	bzero(&(ser_addr.sin_zero), 8);

	// Open local file to read the data
	if((fp = fopen ("myfile.txt","r+t")) == NULL)		  
	{
		printf("File doesn't exit.\n");
		exit(0);
	}
	
	// Perform the transmission and receiving
	ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, &len);			
	
	if (ti != -1)
	{
		// Calculate the average transmission rate
		rt = (len/(float)ti);			
		printf("Ave Time(ms) : %.3f, Ave Data sent(byte): %d\nAve Data rate: %f (Kbytes/s).\n", ti, (int)len, rt);
	}

	close(sockfd);
	fclose(fp);
	exit(0);
}

float str_cli(FILE *fp, int sockfd, struct sockaddr *ser_addr, long *len)
{
	char *buf;
	long lsize,ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	double random_probability = 0.0;
	struct timeval sendt, recvt;
	struct sockaddr_in client_addr;
	int addrlen = sizeof (struct sockaddr_in);
	
	ci = 0;
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes.\n", (int)lsize);
	printf("the packet length is %d bytes.\n",DATALEN);
	
	// Allocate memory to contain the whole file
	buf = (char *) malloc (lsize);			 
	if (buf == NULL) 
		exit (2);
	
	// Copy the file into the buffer
	fread (buf,1,lsize,fp);			  					
	
	// Append the null character at the end of the buffer
	buf[lsize] ='\0';					
	gettimeofday(&sendt,NULL);			
	int c = 0;
	srand (time(NULL));
	
	while (ci <= lsize)
	{
		printf("c = %d\n",c);
		c++;
		if ((lsize+1-ci) <= DATALEN)
			slen = lsize+1-ci;
		else 
			slen = DATALEN;
			
		memcpy(sends, (buf+ci), slen);
		
		random_probability = (double)rand() / (double)RAND_MAX;
		if (random_probability > ERROR_PROBABILITY)			
		{
			// Send a packet without error
			n = sendto(sockfd, &sends, slen, 0, ser_addr, sizeof (struct sockaddr_in));
			if (n == -1) 
			{
				printf("Error sending data packet.\n");								
				exit(1);
			}
		}
		else			
		{
			// Send a packet with error
			n = sendto(sockfd, &sends, BAD_PACKET_LENGTH, 0, ser_addr, sizeof (struct sockaddr_in));
			if (n == -1) 
			{
				printf("Error sending data packet.\n");								
				exit(1);
			}
		}
		
		// Receive ACK or NACK
		if ((n= recvfrom(sockfd, &ack, 2, 0, (struct sockaddr *)&client_addr, &addrlen)) == -1) 		
		{	        
			printf("Error receiving ACK or NACK.\n");
			exit(1);
		}
	
		// If received NACK, continue and retransmit last packet.
		if ((ack.num == NACK_CODE) || (ack.len != 0))         		
		{
			printf("Error in transmission. Retransmitting packet.\n");
			continue;
		}	
		ci += slen;
	}
		
	gettimeofday(&recvt, NULL);
	*len= ci;                                                         
	tv_sub(&recvt, &sendt);				// get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);	
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}