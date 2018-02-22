#include "main.h"


void *thrd_func_server_discover(){
    unsigned char msg[1];
    msg[0] = BROADCAST;
	isMasterDiscovered = FALSE;
    while(!isMasterDiscovered){
        broadcastServerDiscover.sin_addr.s_addr = inet_addr("192.168.1.218");
        sendto(sock,msg,1,0,(struct sockaddr*)&broadcastServerDiscover,sizeof(broadcastServerDiscover));
        sleep(BROADCAST_DELAY);
    }
    
}



void *thrd_func_offset(void *__slave){
	struct slave_t	*slave  = (struct slave_t *)__slave;
	uint16_t		RTTcount;	
	int8_t			success_count = 0, bestResult, offsetDir, running = 1, offset_fails,res;
	uint8_t			skew_position = 0;
	uint32_t		total_rtt = 0;
	uint64_t		sec;
	float			clockSkew, skewTMP;
	struct timeval		skew_clock, currTime;
	struct SkewData		*currentNode = NULL;

    //------------------Get delays Range---------------------------------------------------
rtt_start:;
    slave->minRTT = 0;
    slave->maxRTT = REQUEST_TIMEOUT;
    offset_fails=0;
    if((RTTcount = getRTT(slave)) == 0) {
		printf("RTT get error\n");
	}
    getRTTInterval(slave, RTTcount);
    printf("Range  %d-%d\n",slave->minRTT,slave->maxRTT);
	printf("--------------------------------------------------\n"); 
    deleteRttChain(slave->rtt);
    //--------------------------------------------------------------------------------	
	while(1){
		if((res = getOffsetOfServer(slave,&sec,&skew_clock)) <=0){
            if(offset_fails>10)goto rtt_start;
            offset_fails++;
            continue;
        }
        
		
        if(slave->skew != NULL){
			if(success_count == SKEW_COUNT){
				slave->skew = deleteFirstNode(slave->skew);
				success_count--;
			}
			currentNode = addNewNode(currentNode,&sec,&skew_clock );
            slave->actualSkew = calculateSkewLR(slave->skew);
			printf("SkewLR %.3f\n", slave->actualSkew);
			success_count++;
		}
		else{
			slave->skew = addNewNode(slave->skew,&sec, &skew_clock);
			currentNode = slave->skew;
			success_count++;
		}
        
        sleep(DELAY);
	}

}
