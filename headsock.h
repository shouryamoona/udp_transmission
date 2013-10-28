#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#define NEWFILE (O_WRONLY|O_CREAT|O_TRUNC)
#define MYTCP_PORT 4950
#define MYUDP_PORT 5350
#define DATALEN 500
#define BUFSIZE 60000
#define ACK_CODE 1
#define NACK_CODE 0
#define ERROR_PROBABILITY 0.20
#define BAD_PACKET_LENGTH 0
#define HEADLEN 8

struct pack_so			
{
uint32_t num;			
uint32_t len;			
char data[DATALEN];		
};

struct ack_so
{
uint8_t num;
uint8_t len;
};
