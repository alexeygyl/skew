#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


#define BUFFSIZE 255
#define BROADCAST_DELAY 2
#define RTT_REQUEST_COUNT 255
#define OFFSET_REQUEST_COUNT 100
#define REQUEST_TIMEOUT 65000
#define	SKEW_COUNT 50
#define DELAY 10

enum bool{
    FALSE,
    TRUE
};

enum mode{
    SERVER,
    CLIENT
};


enum TAGS{
    OFFSET_REQ,
    OFFSET_RES,
    RTT_REQ,
    RTT_RES,
    BROADCAST,
    MASTER
};

#pragma pack(push, 1)

#define ARG "hDp:SC"
struct args{
	uint16_t	port;
    uint8_t     daemon:1;
	uint8_t		mode:7;
}args;

struct request_t{
	uint8_t		received;
	uint32_t	seq;
	struct	timeval	time;
};

struct rtt_chain{
	uint16_t	        value;
	struct rtt_chain	*next;
};

struct slave_t{
	struct request_t    packet;
	uint32_t		    expected_p;
	uint16_t		    maxRTT;
	uint16_t		    minRTT;
	float			    actualSkew;
	struct SkewData		*skew;
	struct 	rtt_chain	*rtt;
};

struct SkewData{
	uint64_t	sec_master;
	struct timeval	offset;
	struct SkewData	*next;
};



#pragma pack (pop)

int32_t             sock;
uint32_t            source_len;
uint8_t			    bytes_r, isMasterDiscovered;
struct sockaddr_in	source,broadcastServerDiscover, master;

unsigned char       rxbuff[BUFFSIZE],txbuff[BUFFSIZE];
pthread_t           thrd_server_discover, thrd_offset;

/* This function reads arguments from command line */
void readAttr(int _argc, char ** _argv);

/*These functions start program in Server and Client mode */
void startClient();
void startServer();

/* This function creates the UDP socket*/
void createUDPSocket(int32_t *__sock, uint16_t  *__port);

void initBroadcastMessage(int *__sock);

/*Thread's functions*/
void *thrd_func_server_discover();
void *thrd_func_offset(void *__slave);



uint16_t getRTT(struct slave_t *__slave);
void getRTTInterval(struct slave_t *__slave, uint16_t __count);
int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2);
struct rtt_chain *addNewRttToChain(struct rtt_chain *__rtt, uint16_t __value);
void printChain(struct rtt_chain *__rrt);
void deleteRttChain(struct rtt_chain *__rtt);
int8_t getOffsetOfServer(struct slave_t *__slave, uint64_t *__sec, struct timeval *__offset);


/*
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stddef.h>


#define CLOCK_SYNC_COUNT 10





//---------------------------FUNCTIONS--------------------------------------------------
void timer(struct timeval *__time);
void changeTime(struct timeval *__time, int32_t sec, int32_t usec);

//--------------------OFFSET AND SKEW FUNCTIONS-----------------------------------------
struct timeval getLastOffset(struct SkewData *__skew);
//float getSkewDelta(struct Coluns *__colun, float *__skew);
float getSkewLRDelta(struct Coluns *__colun, float *__skew);
void printSkewMead(struct SkewData *__skew);
float calculateSkewTime(struct SkewData *__skew);
float calculateSkewLR(struct SkewData *__skew);
struct SkewData *addNewNode(struct SkewData *__current, uint64_t *__sec, struct timeval *__offset);
struct SkewData *deleteFirstNode(struct SkewData *__head);
*/

