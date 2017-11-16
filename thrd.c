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
	struct slave_t	    *slave  = (struct slave_t *)__slave;
	int8_t			    success_count = 0;
    uint8_t             est = 0;
	struct timeval		skew_clock;
	struct SkewData		*currentNode = NULL;
    struct data_entry   *entry;
    struct data_t       *curr;
    //------------------Get offset and skew---------------------------------------------------
	correctSkew = 0.0;
    while(1){
        slave->minRTT = 0;
        slave->maxRTT = REQUEST_TIMEOUT;
        
        fixTime();/* Fixing  time*/
        entry = collectData(slave);/*Collect data*/
        getRTTInterval(slave, entry); /*Calculate interval*/
        skew_clock.tv_usec = calcOffset(slave, entry);/*Calculate offset with base on interval*/
        //printf("Range  %d-%d\n",slave->minRTT,slave->maxRTT);
        
        if(slave->skew != NULL){
			currentNode = addNewNode(currentNode,&entry->time,&skew_clock );
            slave->actualSkew = calculateSkewLR(slave->skew);/*calculate skew*/
            //correctSkew +=slave->actualSkew;
			//printf("SkewLR %.3f\n", slave->actualSkew);
			success_count++;
		}
		else{
			slave->skew = addNewNode(slave->skew,&entry->time, &skew_clock);
			currentNode = slave->skew;
			success_count++;
		}
        if(success_count == SKEW_COUNT){
            slave->skew = deleteFirstNode(slave->skew);
            success_count--;
        }
        if(correctSkew != 0.0)printf("%"PRIu64"\t%d\t%.3f\t%.3f\n",currentNode->sec_master
                                                ,currentNode->offset.tv_usec
                                                ,slave->actualSkew
                                                ,correctSkew);
        
        if(success_count > 5 ){
            
            /*if(est == 1){
                if(slave->actualSkew < 0.4 & slave->actualSkew > -0.4)correctSkew +=slave->actualSkew*2;
            }
            else{ 
                if(slave->actualSkew < 0.4 && slave->actualSkew > -0.4)est = 1;
                correctSkew += slave->actualSkew;
        
            }*/
        }
        else if(success_count  == 5 && correctSkew == 0.0){
            correctSkew +=slave->actualSkew;
            while(slave->skew){
                currentNode = slave->skew;
                slave->skew = slave->skew->next;
                free(currentNode);
            }            
            success_count = 0 ;
        }
                   
        /*
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
        while(entry->head){
            curr = entry->head;
            entry->head = entry->head->next;
            free(curr);
        }
        free(entry);
        sleep(DELAY);
	}

}
