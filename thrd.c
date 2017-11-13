#include "main.h"


void *thrd_func_server_discover(){
    unsigned char msg[1];
    msg[0] = BROADCAST;
	isMasterDiscovered = FALSE;
    while(!isMasterDiscovered){
        broadcastServerDiscover.sin_addr.s_addr = inet_addr("192.168.1.200");
        sendto(sock,msg,1,0,(struct sockaddr*)&broadcastServerDiscover,sizeof(broadcastServerDiscover));
        sleep(BROADCAST_DELAY);
    }
    
}

void *thrd_func_correct_time(){
    struct timeval tstamp;
    while(1){
        gettimeofday(&tstamp,NULL);
        tstamp.tv_usec -=  (int)correctSkew;
        if((int)tstamp.tv_usec >= 1000000){
            tstamp.tv_sec++;
            tstamp.tv_usec -=1000000;
        }
        else if((int)tstamp.tv_usec < 0){
            tstamp.tv_sec--;
            tstamp.tv_usec +=1000000;
        }
        settimeofday(&tstamp,NULL);
        //printf("Corrected skew %.3f: %d.%d\n",correctSkew,tstamp.tv_sec, tstamp.tv_usec);
        sleep(1);
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
    struct timeval tstamp;
    uint8_t         est = 0;
    struct data_entry   *entry;
    struct data_t       *curr;
    //------------------Get delays Range---------------------------------------------------
         /*
rtt_start:;

    slave->minRTT = 0;
    slave->maxRTT = REQUEST_TIMEOUT;
    offset_fails=0;
    if((RTTcount = getRTT(slave)) == 0) {
		printf("RTT get error\n");
	}
    exit(0);
    getRTTInterval(slave, RTTcount);
    printf("Range  %d-%d\n",slave->minRTT,slave->maxRTT);
	printf("--------------------------------------------------\n"); 
    deleteRttChain(slave->rtt);
    goto rtt_start;
    */
    //--------------------------------------------------------------------------------	
	correctSkew = 14.5;
    while(1){
        
        entry = collectData(slave);
        getRTTInterval(slave, RTTcount);
        printf("Range  %d-%d\n",slave->minRTT,slave->maxRTT);

        while(entry->head){
            curr = entry->head;
            entry->head = entry->head->next;
            free(curr);
        }
        printf("TIME %d\n",entry->time);
        free(entry);
        
        
        /* Fixing  time */
        /*
        gettimeofday(&tstamp,NULL);
        tstamp.tv_usec -=  (int)(correctSkew*DELAY);
        //printf("TO fix %d\n",(int)(correctSkew*DELAY));
        if((int)tstamp.tv_usec >= 1000000){
            tstamp.tv_sec++;
            tstamp.tv_usec -=1000000;
        }
        else if((int)tstamp.tv_usec < 0){
            tstamp.tv_sec--;
            tstamp.tv_usec +=1000000;
        }
        settimeofday(&tstamp,NULL);

		if((res = getOffsetOfServer(slave,&sec,&skew_clock)) <=0){
            printf("OCCUREd ERROR WHILE DETECTING OFFSET\n");
            offset_fails++;
            //if(offset_fails>10)goto rtt_start;
            goto todelay;
        }

        if(slave->skew != NULL){
			currentNode = addNewNode(currentNode,&sec,&skew_clock );
            slave->actualSkew = calculateSkewLR(slave->skew);
			//printf("SkewLR %.3f\n", slave->actualSkew);
			success_count++;
		}
		else{
			slave->skew = addNewNode(slave->skew,&sec, &skew_clock);
			currentNode = slave->skew;
			success_count++;
		}
        if(success_count == SKEW_COUNT){
            
            if(est == 0){
                if(slave->actualSkew > 0.1 || slave->actualSkew< -0.1){
			        correctSkew+=slave->actualSkew;
                }
                else est = 1;
            }
            printf("%"PRIu64"\t%d\t%d\t%.3f\t%.3f\n",currentNode->sec_master
                                                        ,calculateMedia(slave)
                                                        ,calculateLastEstimateOffset(slave->skew,currentNode,calculateAlhaLR(slave),slave->actualSkew)
                                                        ,slave->actualSkew
                                                        ,correctSkew);
            while(slave->skew){
                currentNode = slave->skew;
                slave->skew = slave->skew->next;
                free(currentNode);
            }
            
            success_count = 0 ;
		}
        */
        sleep(DELAY);
	}

}
