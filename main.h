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
#define OFFSET_REQUEST_COUNT 200
#define REQUEST_TIMEOUT 65000
#define	SKEW_COUNT 10
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


struct slave_t{
	struct request_t    packet;
	uint32_t		    expected_p;
	uint16_t		    maxRTT;
	uint16_t		    minRTT;
	float			    actualSkew;
	struct SkewData		*skew;
};

struct SkewData{
	uint64_t	sec_master;
	struct timeval	offset;
	struct SkewData	*next;
};

struct data_entry{
    struct data_t   *head;
    struct data_t   *tail;
    uint64_t        time;
    uint16_t        count;
};

struct data_t{
    uint16_t        rtt;
    uint32_t        offset;
    struct data_t   *next;
};



#pragma pack (pop)
float               correctSkew;
int32_t             sock;
uint32_t            source_len;
uint8_t			    bytes_r, isMasterDiscovered;
struct sockaddr_in	source,broadcastServerDiscover, master;

unsigned char       rxbuff[BUFFSIZE],txbuff[BUFFSIZE];
pthread_t           thrd_server_discover, thrd_offset, thrd_correct_time;

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
void *thrd_func_correct_time();



void getRTTInterval(struct slave_t *__slave, struct data_entry *__entry);
int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2);
uint32_t calcOffset(struct slave_t *__slave, struct data_entry *__entry);
struct data_entry *collectData();

int32_t calculateMedia(struct slave_t *__slave);
int32_t calculateAlhaLR(struct slave_t *__slave);
float calculateSkewLR(struct SkewData *__skew);
struct SkewData *addNewNode(struct SkewData *__current, uint64_t *__sec, struct timeval *__offset);
struct SkewData *deleteFirstNode(struct SkewData *__head);
void freeSkewData(struct SkewData *__head);
int32_t calculateLastEstimateOffset(struct SkewData *__head, struct SkewData *__tail, int32_t __alpha, float __beta);


