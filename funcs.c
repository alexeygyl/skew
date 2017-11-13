#include "main.h"



//###########################################################################################################################

int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2){
	if(*__usec2 >= *__usec1)return *__usec2 - *__usec1;
	else return 1000000 - *__usec1 + *__usec2;
}

//###########################################################################################################################

struct data_entry *collectData(struct slave_t *__slave){
    uint32_t		i;
	struct timeval	start,end;
	uint16_t		rtt,count = 0;
	uint16_t		pos, res[1000], max=0;
    int16_t         y;
    txbuff[0] = RTT_REQ;
	struct data_entry *entry = (struct data_entry *)malloc(sizeof(struct data_entry));
    entry->head = NULL;
    entry->tail = NULL;
	memset(&res,'\0',1000*2);
	for(i = 1; i <= OFFSET_REQUEST_COUNT; i++){
		memcpy(&txbuff[1],&i,4);
		gettimeofday(&start,NULL);
		sendto(sock,txbuff,5,0, (struct sockaddr*)&master, sizeof(master));
        __slave->expected_p = i;
		__slave->packet.received = FALSE;
		gettimeofday(&end,NULL);
		while((rtt = getmsdiff(&start.tv_usec,&end.tv_usec)) < REQUEST_TIMEOUT){
			if(__slave->packet.received == TRUE){
				__slave->packet.received = FALSE;
				//rtt =  getmsdiff(&start.tv_usec,&end.tv_usec);
				//count++;
                //printf("%d:1\tRTT %d: Time 1 %d.%d, Time 2 %d.%d\n",i,rtt, start.tv_sec,start.tv_usec,end.tv_sec,end.tv_usec);
				
                if(entry->tail != NULL){
                    entry->tail->next = (struct data_t *)malloc(sizeof(struct data_t));
                    entry->tail = entry->tail->next;
                    entry->tail->next = NULL;
                    entry->tail->rtt = rtt;
                   // printf("NOT NULL = %"PRIu16"\n",entry->tail->rtt);
                }
                else{
                    entry->head = (struct data_t *)malloc(sizeof(struct data_t));
                    entry->head->next = NULL;
                    entry->head->rtt = rtt;
                    entry->tail = entry->head;
                   // printf("NULL = %"PRIu16"\n",entry->tail->rtt);
				}

				//pos = rtt/100;
				//if(pos<1000)res[pos]++;
				break;

			}
            gettimeofday(&end,NULL);
		}
    }
    entry->time = end.tv_sec;
    /*
    if(count == 0 )return 0;
    //for(y = 0; y < 1000; y++)if(res[y]!=0)printf("%d-%d\t%d\n",y*100,y*100+99,res[y]);	    

    for(y = 0; y < 1000; y++){
		if(res[y]*100/count >=5){
			__slave->minRTT = y*100;
			break;
		}
	}
	for(y = 999; y >=0; y--){
        if(res[y]*100/count >=5){
			__slave->maxRTT = y*100+99;
			break;
		}
	}
    */
return entry;
}


//###########################################################################################################################
uint16_t getRTT(struct slave_t *__slave){
    uint32_t		i;
struct timeval	start,end;
	uint16_t		rtt,count = 0;
	uint16_t		pos, res[1000], max=0;
    int16_t         y;
    txbuff[0] = RTT_REQ;
	struct rtt_chain	*currRtt = NULL;
	memset(&res,'\0',1000*2);
	for(i = 1; i <= RTT_REQUEST_COUNT; i++){
		memcpy(&txbuff[1],&i,4);
		gettimeofday(&start,NULL);
		sendto(sock,txbuff,5,0, (struct sockaddr*)&master, sizeof(master));
        __slave->expected_p = i;
		__slave->packet.received = FALSE;
		gettimeofday(&end,NULL);
		while(getmsdiff(&start.tv_usec,&end.tv_usec) < REQUEST_TIMEOUT){
			if(__slave->packet.received == TRUE){
				__slave->packet.received = FALSE;
				rtt =  getmsdiff(&start.tv_usec,&end.tv_usec);
				count++;
                printf("%d:2\tRTT %d: Time 1 %d.%d, Time 2 %d.%d\n",i,rtt, start.tv_sec,start.tv_usec,end.tv_sec,end.tv_usec);
					
                if(__slave->rtt !=NULL){
                    currRtt = addNewRttToChain(currRtt,rtt);
                }
                else{
					__slave->rtt = addNewRttToChain(NULL,rtt);
					currRtt = __slave->rtt;
				}

				pos = rtt/100;
				if(pos<1000)res[pos]++;
				break;

			}
			gettimeofday(&end,NULL);
		}
        
	}
    if(count == 0 )return 0;
    //for(y = 0; y < 1000; y++)if(res[y]!=0)printf("%d-%d\t%d\n",y*100,y*100+99,res[y]);	    

    for(y = 0; y < 1000; y++){
		if(res[y]*100/count >=5){
			__slave->minRTT = y*100;
			break;
		}
	}
	for(y = 999; y >=0; y--){
        if(res[y]*100/count >=5){
			__slave->maxRTT = y*100+99;
			break;
		}
	}
	return count;

}

//###########################################################################################################################
//---------------------------------------------------------------------------------------------------------------------------------------
struct rtt_chain *addNewRttToChain(struct rtt_chain *__rtt, uint16_t __value){
	struct rtt_chain *newNode = (struct rtt_chain *)malloc(sizeof(struct rtt_chain));
	newNode->value = __value;
	newNode->next = NULL;
	if(__rtt !=NULL){
		__rtt->next = newNode;
		return newNode;
	}
	else return newNode;
}

void printChain(struct rtt_chain *__rtt){
    while(__rtt){
        printf("RTT = %d\n",__rtt->value);
        __rtt = __rtt->next;
    }
}

void deleteRttChain(struct rtt_chain *__rtt){
    if(__rtt){
        deleteRttChain(__rtt->next);
        free(__rtt);
        __rtt = NULL;
    }
}


//###########################################################################################################################
uint16_t RTT_MMM(struct slave_t *__slave){
	uint32_t	total = 0, max = 0, min = REQUEST_TIMEOUT, count = 0;
	uint16_t	out;
    struct rtt_chain *currRtt = __slave->rtt;
	while(currRtt){
		if(currRtt->value > __slave->minRTT && currRtt->value < __slave->maxRTT){
			max = max > currRtt->value?max:currRtt->value;
			min = min < currRtt->value?min:currRtt->value;
			total += currRtt->value;
			count ++;
		}
        currRtt = currRtt->next;
	}
	out = count; 
	return out;
}

//###########################################################################################################################
//---------------------------------------------------------------------------------------------------------------------------------------
uint32_t rttAverage(struct slave_t *__slave){
    uint32_t	total,count;
	struct rtt_chain *currRtt = __slave->rtt;
    total = 0;
	count = 0;
    while(currRtt){	
		if(currRtt->value>__slave->minRTT && currRtt->value < __slave->maxRTT){
			total += currRtt->value;
            count++;
		}
	    currRtt = currRtt->next;
	}
	return total/count;
}
//###########################################################################################################################
//---------------------------------------------------------------------------------------------------------------------------------------
uint32_t rttDesvision(struct slave_t *__slave, uint32_t __avr){
    double	total, count;
	total = 0;
	count = 0;
	struct rtt_chain *currRtt = __slave->rtt;
    while(currRtt != NULL){
		if(currRtt->value > __slave->minRTT && currRtt->value < __slave->maxRTT){
			total += (currRtt->value - __avr)*(currRtt->value - __avr);
			count ++;
		}
        currRtt = currRtt->next;
	}
	return (uint32_t)sqrt(total/count);
}

//###########################################################################################################################
void getRTTInterval(struct slave_t *__slave, uint16_t __count){
    int i;
	uint32_t	average, desvision;
	uint16_t	currRTTcount;
	for(i = 0; i < 10;i++){
		average = rttAverage(__slave);
		desvision = rttDesvision(__slave,average );
        
        __slave->minRTT = average <=desvision*1.5? 0 : average - desvision*1.5;
        __slave->maxRTT = average + desvision*1.5;
        
        currRTTcount = RTT_MMM(__slave);
	    //printf("RANGE: %d-%d, %d%\n",__slave->minRTT,__slave->maxRTT,currRTTcount*100/__count);	
		if(currRTTcount*100/__count < 65)break;
	}

}

//###########################################################################################################################


int8_t getoffset(struct timeval *__master, struct timeval *__slave, struct timeval *__offset,uint16_t *__rtt){	
    __offset->tv_sec = __master->tv_sec - __slave->tv_sec;
	__offset->tv_usec = __master->tv_usec - (__slave->tv_usec - *__rtt/2);
    if((int)__offset->tv_usec < 0){
		__offset->tv_usec += 1000000;
		__offset->tv_sec--;
		return 1;
	}
	return 0;
}

//###########################################################################################################################

int8_t getOffsetOfServer(struct slave_t *__slave, uint64_t *__sec, struct timeval *__offset){
	uint32_t		    total_offset[2], min[2], max[2], i, minN,maxN, totalN;
	uint8_t			    count_offset[2], type, countN;
	struct timeval		tp1,tp2,tv_offset;
	uint16_t		    rtt;
	time_t			    sec[2];
    //uint16_t            res[1000],pos;
    //memset(&res,'\0',2000);
	
    minN = min[0] = min[1] = 1000000;
	maxN = max[0] = max[1] = 0; 
	totalN = total_offset[0] = total_offset[1] = 0;
	countN = count_offset[0] = count_offset[1] = 0;	
	
    for(i = 1; i <= OFFSET_REQUEST_COUNT; i++){
        txbuff[0] = OFFSET_REQ;
		memcpy(&txbuff[1],&i,4);
		gettimeofday(&tp1,NULL);
		sendto(sock,txbuff,5,0, (struct sockaddr*)&master, sizeof(master));
		__slave->expected_p = i;
		__slave->packet.received = FALSE;
		gettimeofday(&tp2,NULL);
		while(getmsdiff(&tp1.tv_usec,&tp2.tv_usec) < REQUEST_TIMEOUT){
			if(__slave->packet.received == TRUE){		
				__slave->packet.received = FALSE;
				rtt =  getmsdiff(&tp1.tv_usec,&tp2.tv_usec);
                //pos = rtt/100;
				//if(pos<1000)res[pos]++;
                if(rtt < __slave->maxRTT + 1000&& rtt > __slave->minRTT){
					type = getoffset(&tp1,&__slave->packet.time,&tv_offset,&rtt);
					sec[type] = tv_offset.tv_sec;      
                    if(minN > (int)tv_offset.tv_usec)minN = tv_offset.tv_usec;
                    if(maxN < (int)tv_offset.tv_usec)maxN = tv_offset.tv_usec;
                    totalN += tv_offset.tv_usec;
                    countN++;
                    
                    //printf("%d:  RTT=%d: master=%d.%d, slave=%d.%d, Offset=%d.%d\n",i,rtt,(int)tp1.tv_sec,(int)tp1.tv_usec,(int)__slave->packet.time.tv_sec,(int) __slave->packet.time.tv_usec,(int)tv_offset.tv_sec,(int)tv_offset.tv_usec);
                }
				break;
			}
			gettimeofday(&tp2,NULL);
		}
	}
    if(countN*100/OFFSET_REQUEST_COUNT< 20)return -1;
    //int y;
    //for(y = 0; y < 1000; y++)if(res[y]!=0)printf("%d: %d-%d\t%d\n",countN,y*100,y*100+99,res[y]);	    
    //if((maxN - minN) > (__slave->maxRTT - __slave->minRTT)*10) return -2;
	*__sec = tp1.tv_sec;
	__offset->tv_usec = (totalN - minN - maxN)/(countN -2);
	__offset->tv_sec = tv_offset.tv_sec;
    return 1;

}
/*
int8_t getOffsetAndRtt(struct slave_t *__slave, uint64_t *__sec, struct timeval *__offset){
	uint32_t		    total_offset[2], min[2], max[2], i, minN,maxN, totalN;
	uint8_t			    count_offset[2], type, countN;
	struct timeval		tp1,tp2,tv_offset;
	uint16_t		    rtt;
	time_t			    sec[2];
    //uint16_t            res[1000],pos;
    //memset(&res,'\0',2000);
	
    minN = min[0] = min[1] = 1000000;
	maxN = max[0] = max[1] = 0; 
	totalN = total_offset[0] = total_offset[1] = 0;
	countN = count_offset[0] = count_offset[1] = 0;	
	
    for(i = 1; i <= OFFSET_REQUEST_COUNT; i++){
        txbuff[0] = OFFSET_REQ;
		memcpy(&txbuff[1],&i,4);
		gettimeofday(&tp1,NULL);
		sendto(sock,txbuff,5,0, (struct sockaddr*)&master, sizeof(master));
		__slave->expected_p = i;
		__slave->packet.received = FALSE;
		gettimeofday(&tp2,NULL);
		while(getmsdiff(&tp1.tv_usec,&tp2.tv_usec) < REQUEST_TIMEOUT){
			if(__slave->packet.received == TRUE){		
				__slave->packet.received = FALSE;
				rtt =  getmsdiff(&tp1.tv_usec,&tp2.tv_usec);
                //pos = rtt/100;
				//if(pos<1000)res[pos]++;
				//type = getoffset(&tp1,&__slave->packet.time,&tv_offset,&rtt);
				//sec[type] = tv_offset.tv_sec;      
                //if(minN > (int)tv_offset.tv_usec)minN = tv_offset.tv_usec;
                //if(maxN < (int)tv_offset.tv_usec)maxN = tv_offset.tv_usec;
                //totalN += tv_offset.tv_usec;
                //countN++;
                    
                    //printf("%d:  RTT=%d: master=%d.%d, slave=%d.%d, Offset=%d.%d\n",i,rtt,(int)tp1.tv_sec,(int)tp1.tv_usec,(int)__slave->packet.time.tv_sec,(int) __slave->packet.time.tv_usec,(int)tv_offset.tv_sec,(int)tv_offset.tv_usec);
				break;
			}
			gettimeofday(&tp2,NULL);
		}
	}
    if(countN*100/OFFSET_REQUEST_COUNT< 20)return -1;
	*__sec = tp1.tv_sec;
	__offset->tv_usec = (totalN - minN - maxN)/(countN -2);
	__offset->tv_sec = tv_offset.tv_sec;
    return 1;

}
*/

int32_t calculateMedia(struct slave_t *__slave){
    uint64_t           total = 0, count = 0;
    struct SkewData *curr;
    if(__slave->skew ==NULL)return 0;
    for(curr = __slave->skew; curr; curr = curr->next, count++){
        total += curr->offset.tv_usec;
    }
    return (int32_t)(total/count);

}

/*
int32_t calculateAlhaLR(struct slave_t *__slave){
    float           Ytotal = 0, Xtotal = 0, count = 0;
    struct timeval  Yfirst;
    uint32_t        Xfirst = 0;
    struct SkewData *curr;
    if(__slave->skew ==NULL)return 0;
    Xfirst = __slave->skew->sec_master;
    Yfirst = __slave->skew->offset;
    for(curr = __slave->skew->next; curr; curr = curr->next, count++){
        Xtotal += (float)(curr->sec_master  - Xfirst);
        Ytotal += (float)(curr->offset.tv_usec - Yfirst.tv_usec);
    }
    return (int32_t)(Ytotal/count - __slave->actualSkew*(Xtotal/count)) + Yfirst.tv_usec;

}

int32_t calculateAlhaLR1(struct slave_t *__slave){
    float           Ytotal = 0, Xtotal = 0, count = 1;
    struct timeval  Yfirst;
    uint32_t        Xfirst = 0;
    struct SkewData *curr;
    if(__slave->skew ==NULL)return 0;
    Xfirst = __slave->skew->sec_master;
    Yfirst = __slave->skew->offset;
    for(curr = __slave->skew->next; curr; curr = curr->next, count++){
        Xtotal += (float)(curr->sec_master  - Xfirst);
        Ytotal += (float)(curr->offset.tv_usec - Yfirst.tv_usec);
    }
    return (int32_t)(Ytotal/count - __slave->actualSkew*(Xtotal/count)) + Yfirst.tv_usec;

}
*/
int32_t calculateAlhaLR(struct slave_t *__slave){
    uint64_t           Ytotal = 0, Xtotal = 0, count = 0, Xfirst;
    struct SkewData *curr;
    Xfirst = __slave->skew->sec_master;
    if(__slave->skew ==NULL)return 0;
    for(curr = __slave->skew; curr; curr = curr->next, count++){
        Xtotal += curr->sec_master - Xfirst;
        Ytotal += curr->offset.tv_usec;
    }
    
    return (int32_t)((float)Ytotal/(float)count - __slave->actualSkew* (float)Xtotal/(float)count);

}

int32_t calculateLastEstimateOffset(struct SkewData *__head, struct SkewData *__tail, int32_t __alpha, float __beta){
    return    __alpha + (int32_t)(__beta*(float)(__tail->sec_master - __head->sec_master));
}
/*
float calculateSkewLR(struct SkewData *__skew){
    float            a;
    float            Ymead, Xmead, XYmead, XXmead;
    int64_t          Ytotal, Xtotal, XYtotal, XXtotal;
    uint8_t          count;
    uint32_t         Xfirst;
    struct timeval   Yfirst;
    struct SkewData *curr;
    Xfirst  = __skew->sec_master;
    Yfirst  = __skew->offset;
    count   = 0;
    Ytotal = Xtotal = XYtotal = XXtotal = 0; 
    for(curr = __skew; curr; curr = curr->next, count++){
        Ytotal  += curr->offset.tv_usec - Yfirst.tv_usec;
        Xtotal  += curr->sec_master - Xfirst;
        XXtotal += (curr->sec_master - Xfirst)*(curr->sec_master - Xfirst);
        XYtotal += (curr->sec_master - Xfirst)*( curr->offset.tv_usec - Yfirst.tv_usec);
    }
    
    Ymead   = (float)Ytotal/(float)count;
    Xmead   = (float)Xtotal/(float)count;
    XYmead  = (float)XYtotal/(float)count;
    XXmead  = (float)XXtotal/(float)count;

    a = (XYmead - Xmead*Ymead)/(XXmead - Xmead*Xmead);
    return a;
}

*/
float calculateSkewLR(struct SkewData *__skew){
    float            a;
    float            Ymead, Xmead, XYmead, XXmead;
    int64_t          Ytotal, Xtotal, XYtotal, XXtotal;
    uint8_t          count;
    uint32_t         Xfirst;
    struct timeval   Yfirst;
    struct SkewData *curr;
    Xfirst  = __skew->sec_master;
    count   = 0;
    Ytotal = Xtotal = XYtotal = XXtotal = 0; 
    for(curr = __skew; curr; curr = curr->next, count++){
        Ytotal  += curr->offset.tv_usec;
        Xtotal  += curr->sec_master - Xfirst;
        XXtotal += (curr->sec_master - Xfirst)*(curr->sec_master - Xfirst);
        XYtotal += (curr->sec_master - Xfirst)*curr->offset.tv_usec;
    }
    
    Ymead   = (float)Ytotal/(float)count;
    Xmead   = (float)Xtotal/(float)count;
    XYmead  = (float)XYtotal/(float)count;
    XXmead  = (float)XXtotal/(float)count;

    a = (XYmead - Xmead*Ymead)/(XXmead - Xmead*Xmead);
    return a;
}


//###########################################################################################################################

struct SkewData *addNewNode(struct SkewData *__current, uint64_t *__sec, struct timeval *__offset){
	struct SkewData	*newNode;
	newNode = (struct SkewData*)malloc(sizeof(struct SkewData));
	newNode->sec_master = *__sec;
	newNode->offset = *__offset;
	newNode->next = NULL;
	if(__current!=NULL){
		__current->next = newNode;
		return newNode;
	}
	else{
		return newNode;
	}
	
}

void freeSkewData(struct SkewData *__head){
    if(__head){
        freeSkewData(__head->next);
        free(__head);
        __head = NULL;
    }
}

//###########################################################################################################################
struct SkewData *deleteFirstNode(struct SkewData *__head){
	struct SkewData *secondNode = __head->next;
    memset(__head,'\0',sizeof(struct SkewData));
	free(__head);
	return secondNode;
}


//###########################################################################################################################
/*
void timer(struct timeval *__time){
		struct timeval tp;
		gettimeofday(&tp,NULL);
		while(__time->tv_sec > tp.tv_sec)gettimeofday(&tp,NULL);
		while(__time->tv_usec > tp.tv_usec)gettimeofday(&tp,NULL);
}
*/
