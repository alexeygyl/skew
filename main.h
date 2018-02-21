
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

enum bool{
    NO,
    YES
};

enum mode{
    SERVER,
    CLIENT
};


enum TAGS{
    OFFSET_REQ,
    OFFSET_RES,
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


#pragma pack (pop)

int32_t             sock;
uint32_t            source_len;
uint8_t			    bytes_r;
struct sockaddr_in	source,broadcastServerDiscover, master;

unsigned char       rxbuff[BUFFSIZE],txbuff[BUFFSIZE];
pthread_t           thrd_server_discover;

/* This function reads arguments from command line */
void readAttr(int _argc, char ** _argv);

/*These functions start program in Server and Client mode */
void startClient();
void startServer();

/* This function creates the UDP socket*/
void createUDPSocket(int32_t *__sock, uint16_t  *__port);

void initBroadcastMessage(int *__sock);

void *thrd_func_server_discover();

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

#include <alsa/asoundlib.h>

#define MUSIC_PATH "/home/music"

#define UNIX_SOCKET "/run/tese.sock"
#define	BUFFSIZE 1026
#define	BUFFSIZE_U 100

#define INIT_REQUEST 0x01                                                                                     
//#define INIT_RESPONCE 0x02                                                                                    
#define OFFSET_RESPONCE 0x03                                                                                  
#define OFFSET_REQUEST 0x04
#define RTT_RESPONCE 0x05                                                                                  
#define RTT_REQUEST 0x06
#define CLOCK_SYNC_REQUEST 0x10
#define CLOCK_SYNC_RESPONCE 0x11
#define LOST_PACKETS 0x12
#define BROADCAST 0xff

#define PREPARE 0xa3
#define PLAY 0xa0
#define STOP 0xa1
#define PAUSE 0xa2
#define VOLUME 0xa4
#define SET 0xa5
#define DATA 0xa9
#define SKEW 0xa8

#define PLAY_S 200
#define STOP_S 201
#define PAUSE_S 202
#define VOLUME_S 203
#define SET_S 204


#define MUSIC_LIST 210
#define ANDROID 211

#define DURATION 230

#define RESEND_PACKET_DELAY 200000
#define COLUNSIZE 8
#define START_SYNC 0xff
#define OFFSET_REQUEST_COUNT 100
#define RTT_REQUEST_COUNT 65000
#define	SKEW_COUNT 50
#define OFFSET_REQUEST_TIMEOUT 65000
#define RTT_REQUEST_TIMEOUT 65000
#define DELAY 10
#define MAX_RTT 10000
#define CLOCK_SYNC_COUNT 10
#define COLUNS_OFFLINE_TIMEOUT 10
#define COLUNS_STOP_SERVICE_TIMEOUT 5
#define MAX_COLUNS 7

#define TRUE 1
#define FALSE 0
#define YES 1
#define NO 0
#define ERROR -1

#define STATUS_NONE 0
#define STATUS_OFFSET 1
#define STATUS_DATA 2





typedef struct{
	uint8_t		status;
	uint32_t	seq;
	struct	timeval	time;
}Request_offset;

struct SkewData{
	uint64_t	sec_master;
	struct timeval	offset;
	struct SkewData	*next;
};

struct RTTGausa{
	uint16_t	rtt;
	struct RTTGausa	*next;
};



struct Coluns{
    pthread_t           thrdMain;
    pthread_t           thrdData;
	struct sockaddr_in	sock;
	Request_offset		packet;
	uint32_t		    expected_p;
	uint16_t		    maxRTT;
	uint16_t		    minRTT;
	float			    actualSkew;
    float               actualSkewLR;
    int8_t              newSkew;
	struct SkewData		*skew;
	struct RTTGausa		*rttGausa;
	enum acousticSystem	channel;
	int32_t			    status;
	uint8_t 			lostS;
	uint32_t    		lostP[256];
    struct timeval    online;
    uint8_t             isPrepared:1;
    uint8_t             isReadyToPlay:1;
    uint8_t             isLost:6;
    struct Coluns       *next;
    struct Coluns       *pref;
};
struct Coluns *colunsHead, *colunsLast;




struct sockaddr_un	serv_unix;
int8_t			    colunsCount;
int32_t			    sock, client_len, sock_unix, fileSize;
struct sockaddr_in	client, broadcastHeartBeat, android;
unsigned char		buff[BUFFSIZE],request[5], buff_play[9],rttRequest[5],buff_skew[5], buff_duration[3];
int8_t			    bytes_r, isExit;
pthread_t		    thrd, action_thrd,skew_thrd, thrd_coluns_detect;
pthread_t           thrd_duration, thrd_for_start_play, thrd_for_watch_dir;
struct timeval		tstart,tend, timeToStart;
char			    isSendingData, isReadyToStart;
music_t         	fileList;
struct MusicFiles   *music;
uint32_t            musicId;
uint8_t	            playingStatus;
uint16_t            musicToPlay, musicCurrentPosition;
//struct timeval	    colunOffset, timeMaster, timeSlave;
int keep_running;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------FUNCTIONS--------------------------------------------------
//void printTree(struct Coluns *__colun);
struct Coluns *addNewColun(struct Coluns *__colun ,struct sockaddr_in *__colunAddr, int8_t __channel);
struct Coluns *getColunByAddr(struct Coluns *__colun ,struct sockaddr_in *__colunAddr);
int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2);
int8_t isReadyToSendSkew();
void timer(struct timeval *__time);
void changeTime(struct timeval *__time, int32_t sec, int32_t usec);
//---------------------SOCKET_FUNCTIONS------------------------------------------------
void createUDPSocket(int32_t *__sock, uint16_t  *__port);
void createUNIXSocket();
void initBroadcastMessage(int *__sock);

//---------------------TREAHD_FUNCTIONS-------------------------------------------------
void *thrd_func_for_offset(void *__colun);
void *thrd_func_for_send_data(void *__colun);
void *action_thrd_func();
void *thrd_func_broadcast_coluns_detect();
void *thrd_func_for_send_skew();
void *thrd_func_send_second_to_client();
void *thrd_func_start_play_when_is_ready();
void *thrd_func_for_watch_dir();
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
uint16_t getRTT(struct Coluns *__colun, uint16_t *__min, uint16_t *__max);
struct RTTGausa *addNewNodeRTTGausa(struct RTTGausa *__current, uint16_t __rtt);
uint16_t RTT_MMM(struct RTTGausa *__head, uint16_t __min, uint16_t __max);
uint32_t rttGausaAverage(struct RTTGausa *__head, uint16_t __min, uint16_t __max, uint8_t __status);
uint32_t rttGausaDesvision(struct RTTGausa *__head, uint32_t __avr, uint16_t __min, uint16_t __max, uint8_t __status);
void rttGausaEleminateTree(struct RTTGausa *__head);
void getRTTInterval(struct RTTGausa *__head, uint16_t *__min, uint16_t *__max, uint16_t __count);

//--------------------MUSICS FUNCTIONS---------------------------------------------------
music_t music_create();
void sendMusicList(struct MusicFiles *__head);
void sendMusic(struct MusicFiles *__head);
char *getFileName(struct MusicFiles *__files, int __pos);
struct MusicFiles *getMusicFileById(struct MusicFiles *__files, int __pos);
struct MusicFiles *addNewFile(struct MusicFiles *__head, char *__fileName, char *__pwd );
void getFilesNameFromDir(music_t __m,char *__pDir);
void parseMetaMP3(struct MusicFiles *__newNode, char *__fullName, char *__pwd);
uint8_t getFormat(char *__file);
void startPlay();
void stopPlay();
void pausePlay();
void setPlay(uint16_t __pos);
int8_t readWavHead(int32_t __fd, struct wavHead_t *__wavHead);
struct stereoSample_t * parseMP3File(struct wavHead_t *__wavHead, struct MusicFiles *__music);
struct stereoSample_t * parseWAVFile(struct wavHead_t *__wavHead, unsigned char *__fileName);
void getLeftRightChannels(struct stereoSample_t *__samples, struct monoSamples_t *__right, struct monoSamples_t *__left, struct MusicFiles *__music);
int8_t  isReadyToPlay();


//--------------------QUEUE FUNCTIONS---------------------------------------------------
#ifndef __EVENT_QUEUE_H
#define __EVENT_QUEUE_H

struct queue_entry;

struct queue_entry{
    struct queue_entry *next_ptr;
    struct inotify_event inot_ev;
};

typedef struct queue_entry *queue_entry_t;


struct queue_struct{
    struct queue_entry *head;
    struct queue_entry *tail;
};

typedef struct queue_struct *queue_t;

int queue_empty(queue_t q);
queue_t queue_create();
void queue_destroy(queue_t q);
void queue_enqueue(queue_entry_t d, queue_t q);
queue_entry_t queue_dequeue (queue_t q);

#endif
//--------------------INOTIFY FUNCTIONS---------------------------------------------------
#ifndef __INOTIFY_UTILS_H
#define __INOTIFY_UTILS_H


void handle_event (queue_entry_t event);
int read_event (int fd, struct inotify_event *event);
int event_check (int fd);
int process_inotify_events (queue_t q, int fd);
int watch_dir (int fd, const char *dirname, unsigned long mask);
int ignore_wd (int fd, int wd);
int close_inotify_fd (int fd);
int open_inotify_fd ();


#endif

*/
