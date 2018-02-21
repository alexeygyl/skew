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


#define BUFFSIZE 255
#define BROADCAST_DELAY 2
#define RTT_REQUEST_COUNT 255
#define REQUEST_TIMEOUT 65000

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


struct slave_t{
	struct request_t    packet;
	uint32_t		    expected_p;
	uint16_t		    maxRTT;
	uint16_t		    minRTT;
	float			    actualSkew;
	//struct SkewData		*skew;
	//struct RTTGausa		*rttGausa;
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



uint16_t getRTT(struct slave_t *__slave, uint16_t *__min, uint16_t *__max);



/*
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stddef.h>



#define	BUFFSIZE_U 100


#define RESEND_PACKET_DELAY 200000
#define COLUNSIZE 8
#define START_SYNC 0xff
#define OFFSET_REQUEST_COUNT 100
#define	SKEW_COUNT 50
#define OFFSET_REQUEST_TIMEOUT 65000
#define DELAY 10
#define MAX_RTT 10000
#define CLOCK_SYNC_COUNT 10




struct SkewData{
	uint64_t	sec_master;
	struct timeval	offset;
	struct SkewData	*next;
};

struct RTTGausa{
	uint16_t	rtt;
	struct RTTGausa	*next;
};








//---------------------------FUNCTIONS--------------------------------------------------
int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2);
void timer(struct timeval *__time);
void changeTime(struct timeval *__time, int32_t sec, int32_t usec);

//--------------------OFFSET AND SKEW FUNCTIONS-----------------------------------------
struct timeval getLastOffset(struct SkewData *__skew);
//float getSkewDelta(struct Coluns *__colun, float *__skew);
float getSkewLRDelta(struct Coluns *__colun, float *__skew);
void printSkewMead(struct SkewData *__skew);
float calculateSkewTime(struct SkewData *__skew);
float calculateSkewLR(struct SkewData *__skew);
int8_t getOffsetFromColun(struct Coluns *__colun, uint64_t *__sec, struct timeval *__offset);
struct SkewData *addNewNode(struct SkewData *__current, uint64_t *__sec, struct timeval *__offset);
struct SkewData *deleteFirstNode(struct SkewData *__head);

//--------------------INTERVAL FUNCTIONS-----------------------------------------------
struct RTTGausa *addNewNodeRTTGausa(struct RTTGausa *__current, uint16_t __rtt);
uint16_t RTT_MMM(struct RTTGausa *__head, uint16_t __min, uint16_t __max);
uint32_t rttGausaAverage(struct RTTGausa *__head, uint16_t __min, uint16_t __max, uint8_t __status);
uint32_t rttGausaDesvision(struct RTTGausa *__head, uint32_t __avr, uint16_t __min, uint16_t __max, uint8_t __status);
void rttGausaEleminateTree(struct RTTGausa *__head);
void getRTTInterval(struct RTTGausa *__head, uint16_t *__min, uint16_t *__max, uint16_t __count);

*/


