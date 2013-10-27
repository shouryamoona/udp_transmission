/*******************************
client.c: the source file of the client in udp transmission for a large packet
********************************/

#include "headsock.h"

float str_cli(FILE *fp, int sockfd,struct sockaddr *ser_addr, long *len);     //packet transmission fuction
void tv_sub(struct  timeval *out, struct timeval *in);					//calculate the time interval between out and in

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

	if (argc != 2) {
		printf("parameters not match");
	}

	if ((sh=gethostbyname(argv[1]))==NULL) {             				//get host's information
		printf("error when gethostbyname");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);                           		//create the socket
	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}
	
	addrs = (struct in_addr **)sh->h_addr_list;
	printf("canonical name: %s\n", sh->h_name);					//print server's information
	
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
	
	bzero(&(ser_addr.sin_zero), 8);
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYUDP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	

	if((fp = fopen ("myfile.txt","r+t")) == NULL)					//open local file to read the data
	{
		printf("File doesn't exit\n");
		exit(0);
	}

	ti = str_cli(fp, sockfd, (struct sockaddr *)&ser_addr, &len);             //perform the transmission and receiving
	
	if (ti != -1)	{
		rt = (len/(float)ti);                                         		//caculate the average transmission rate
		printf("Ave Time(ms) : %.3f, Ave Data sent(byte): %d\nAve Data rate: %f (Kbytes/s)\n", ti, (int)len, rt);
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
	struct ack_so acks;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	struct sockaddr_in client_addr;
	int addrlen = sizeof (struct sockaddr_in);
	
	ci = 0;
	
	fseek (fp , 0 , SEEK_END);
	lsize = ftell (fp);
	rewind (fp);
	printf("The file length is %d bytes\n", (int)lsize);
	printf("the packet length is %d bytes\n",DATALEN);
	
// allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL) exit (2);
	
  // copy the file into the buffer.
	fread (buf,1,lsize,fp);							

  /*** the whole file is loaded in the buffer. ***/
	buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	
	while(ci<= lsize)
	{
		if ((lsize+1-ci) <= DATALEN)
			slen = lsize+1-ci;
		else 
			slen = DATALEN;
		memcpy(sends, (buf+ci), slen);
		n = sendto(sockfd, &sends, slen, 0, ser_addr, sizeof (struct sockaddr_in));			//send the data
		if(n == -1) {
			printf("send error!");								
			exit(1);
		}
		ci += slen;
	}
	
	if ((n= recvfrom(sockfd, &acks, 2, 0, (struct sockaddr *)&client_addr, &addrlen)) == -1) {	        	//receive ACK or NACK
		printf("error receiving data\n");
		exit(1);
	}
	
	if ((acks.num != 1) && (acks.len != 0))         				
		printf("error in transmission\n");
	
	gettimeofday(&recvt, NULL);
	*len= ci;                                                         //get current time
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
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
