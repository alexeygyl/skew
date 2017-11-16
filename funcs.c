#include "main.h"




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


int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2){
	if(*__usec2 >= *__usec1)return *__usec2 - *__usec1;
	else return 1000000 - *__usec1 + *__usec2;
}

//###########################################################################################################################

struct data_entry *collectData(struct slave_t *__slave){
    uint32_t		i;
	struct timeval	start,end, tv_offset;
	uint16_t		rtt,count = 0;
    int16_t         y;
    txbuff[0] = OFFSET_REQ;
	struct data_entry *entry = (struct data_entry *)malloc(sizeof(struct data_entry));
    entry->head = NULL;
    entry->tail = NULL;
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
				getoffset(&start,&__slave->packet.time,&tv_offset,&rtt);
				count++;
                //printf("%d:1\tRTT %d: Time 1 %d.%d, Time 2 %d.%d\n",i,rtt, start.tv_sec,start.tv_usec,end.tv_sec,end.tv_usec);
				
                if(entry->tail != NULL){
                    entry->tail->next = (struct data_t *)malloc(sizeof(struct data_t));
                    entry->tail = entry->tail->next;
                    entry->tail->next = NULL;
                    entry->tail->rtt = rtt;
                    entry->tail->offset = tv_offset.tv_usec;
                    //printf(" NOT NULL = %d\n",entry->tail->offset);
                }
                else{
                    entry->head = (struct data_t *)malloc(sizeof(struct data_t));
                    entry->head->next = NULL;
                    entry->head->rtt = rtt;
                    entry->head->offset = tv_offset.tv_usec;
                    entry->tail = entry->head;
                    //printf("NULL = %d\n",entry->tail->offset);
				}

				break;

			}
            gettimeofday(&end,NULL);
		}
    }
    entry->time = end.tv_sec;
    entry->count = count;
return entry;
}



//###########################################################################################################################
uint8_t getProbability(struct slave_t *__slave, struct data_entry *__entry){
	uint16_t	out = 0;
    struct data_t   *curr = __entry->head;
	while(curr){
		if(curr->rtt > __slave->minRTT && curr->rtt < __slave->maxRTT){
			out++;
		}
        curr = curr->next;
	}
	return out*100/__entry->count;
}

//###########################################################################################################################
//---------------------------------------------------------------------------------------------------------------------------------------
uint32_t rttAverage(struct slave_t *__slave,struct data_t *__head){
    uint32_t	total,count;
	struct data_t   *curr = __head;
    total = 0;
	count = 0;
    while(curr){
		if(curr->rtt > __slave->minRTT && curr->rtt < __slave->maxRTT){
			total += curr->rtt;
            count++;
		}
	    curr = curr->next;
	}
    if(count == 0)return 0;
	return total/count;
}
//###########################################################################################################################
//---------------------------------------------------------------------------------------------------------------------------------------
uint32_t rttDesvision(struct slave_t *__slave,struct data_t *__head , uint32_t __avr){
    double	total, count;
	total = 0;
	count = 0;
	struct data_t   *curr = __head;
    while(curr != NULL){
		if(curr->rtt > __slave->minRTT && curr->rtt < __slave->maxRTT){
			total += (curr->rtt - __avr)*(curr->rtt - __avr);
			count ++;
		}
        curr = curr->next;
	}
    if(count ==0 )return 0;
	return (uint32_t)sqrt(total/count);
}

//###########################################################################################################################
void getRTTInterval(struct slave_t *__slave, struct data_entry *__entry){
    int i;
	uint32_t	average, desvision;
	uint16_t	currRTTcount;
	for(i = 0; i < 10;i++){
		average = rttAverage(__slave,__entry->head);
        desvision = rttDesvision(__slave,__entry->head,average );
        
        __slave->minRTT = average <=desvision*1.5? 0 : average - desvision*1.5;
        __slave->maxRTT = average + desvision*1.5;
        
	    //printf("RANGE: %d-%d, %d%\n",__slave->minRTT,__slave->maxRTT, getProbability(__slave, __entry));	
		if(getProbability(__slave, __entry) < 65)break;
	}

}


//###########################################################################################################################


uint32_t calcOffset(struct slave_t *__slave, struct data_entry *__entry){
	uint32_t         min,max, total, count;
	struct data_t   *curr;
    min  = 1000000;
	max  = total = count =  0;	
	
    for(curr = __entry->head; curr; curr = curr->next){
        if(curr->rtt > __slave->minRTT && curr->rtt < __slave->maxRTT){
            if(min > curr->offset )min = curr->offset;
            if(max < curr->offset )max = curr->offset;
            total+=curr->offset; 
            count++;
        }
    }
    
   
	return (total - min - max)/(count -2);

}

int32_t calculateMedia(struct slave_t *__slave){
    uint64_t           total = 0, count = 0;
    struct SkewData *curr;
    if(__slave->skew ==NULL)return 0;
    for(curr = __slave->skew; curr; curr = curr->next, count++){
        total += curr->offset.tv_usec;
    }
    return (int32_t)(total/count);

}

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


float calculateSkewLR(struct SkewData *__skew){
    float            a;
    float            Ymead, Xmead, XYmead, XXmead;
    int64_t          Ytotal, Xtotal, XYtotal, XXtotal;
    uint8_t          count;
    uint64_t         Xfirst;
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


void fixTime(){
    struct timeval tstamp;
    gettimeofday(&tstamp,NULL);
    tstamp.tv_usec -=  (int)(correctSkew*DELAY);
    if((int)tstamp.tv_usec >= 1000000){
        tstamp.tv_sec++;
        tstamp.tv_usec -=1000000;
    }
    else if((int)tstamp.tv_usec < 0){
        tstamp.tv_sec--;
        tstamp.tv_usec +=1000000;
    }
    settimeofday(&tstamp,NULL);
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
