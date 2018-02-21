#include "main.h"



//###########################################################################################################################

int32_t getmsdiff(suseconds_t *__usec1, suseconds_t *__usec2){
	if(*__usec2 >= *__usec1)return *__usec2 - *__usec1;
	else return 1000000 - *__usec1 + *__usec2;
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
				if(i == __slave->packet.seq){
					rtt =  getmsdiff(&start.tv_usec,&end.tv_usec);
					count++;
                	//printf("%d:\tRTT %d: Time 1 %d.%d, Time 2 %d.%d\n",i,rtt, start.tv_sec,start.tv_usec,end.tv_sec,end.tv_usec);
					
                    if(__slave->rtt !=NULL)currRtt = addNewRttToChain(currRtt,rtt);
					else{
					    __slave->rtt = addNewRttToChain(NULL,rtt);
						currRtt = __slave->rtt;
					}

					pos = rtt/100;
					if(pos<1000)res[pos]++;
				    break;
                }

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
				if(i == __slave->packet.seq){
					__slave->packet.received = FALSE;
					rtt =  getmsdiff(&tp1.tv_usec,&tp2.tv_usec);
					
                    if(rtt < __slave->maxRTT && rtt > __slave->minRTT){
						type = getoffset(&tp1,&__slave->packet.time,&tv_offset,&rtt);
						sec[type] = tv_offset.tv_sec;      
                        if(minN > (int)tv_offset.tv_usec)minN = tv_offset.tv_usec;
                        if(maxN < (int)tv_offset.tv_usec)maxN = tv_offset.tv_usec;

                        totalN += tv_offset.tv_usec;
						countN++;
                       printf("%d:  RTT=%d: master=%d.%d, slave=%d.%d, Offset=%d.%d\n",i,rtt,(int)tp1.tv_sec,(int)tp1.tv_usec,(int)__slave->packet.time.tv_sec,(int) __slave->packet.time.tv_usec,(int)tv_offset.tv_sec,(int)tv_offset.tv_usec);
                    }
					break;
				}else{
					__slave->packet.received = 0;
				}
			}
			gettimeofday(&tp2,NULL);
		}
	}
    if(countN*100/OFFSET_REQUEST_COUNT< 20)return -1;
    if((maxN - minN) > (__slave->maxRTT - __slave->minRTT)*10) return -2;
	*__sec = tp1.tv_sec;
	__offset->tv_usec = (totalN - minN - maxN)/(countN -2);
	__offset->tv_sec = tv_offset.tv_sec; 
    return 1;

}

/*

//###########################################################################################################################

float getSkewDelta(struct Coluns *__colun, float *__skew){
	static float	total = 0.0, count = 0.0;
    float           skew;
	if(colunsCount <= 1)return 0.0;
    if(__colun != NULL){
		total+=__colun->actualSkew;
        count++;
        return getSkewDelta(__colun->next,__skew);
	}
    skew =  *__skew - total/count;	
    count = total = 0.0;
	return skew;
}

//###########################################################################################################################
float getSkewLRDelta(struct  Coluns *__colun, float *__skew){
	static float	total = 0.0, count = 0.0;
    float           skew;
    if(colunsCount <= 1)return 0.0;
    if(__colun != NULL){
		total+=__colun->actualSkewLR;
        count++;
        return getSkewLRDelta(__colun->next,__skew);
	}
    
	skew = total/count - *__skew;	
    //printf("Total %.3f, count %.3f, __Skew %.3f, SKEW %.3f\n",total, count,*__skew,skew);
    count = total = 0.0;
	return skew;	
}

//###########################################################################################################################

float calculateSkewLR(struct SkewData *__skew){
    float                   a;
    float                   Ymead, Xmead, XYmead, XXmead;
    static int64_t          Ytotal, Xtotal, XYtotal, XXtotal;
    static uint8_t          count;
    static uint32_t         Xfirst;
    static struct timeval   Yfirst;
    if(Xfirst == 0 ){
        Xfirst    = __skew->sec_master;
        Yfirst  = __skew->offset;
        count       = 1;
        Ytotal = Xtotal = XYtotal = XXtotal = 0;
        if(__skew->next != NULL)return calculateSkewLR(__skew->next);
        return 0;
    }
   //printf("-------------------------------------------------------------------------\n"); 
    
    count++;
    Ytotal  += __skew->offset.tv_usec - Yfirst.tv_usec;
    Xtotal  += __skew->sec_master - Xfirst;
    XXtotal += (__skew->sec_master - Xfirst)*(__skew->sec_master - Xfirst);
    XYtotal += (__skew->sec_master - Xfirst)*( __skew->offset.tv_usec - Yfirst.tv_usec);
    
    if(__skew->next != NULL){
        return calculateSkewLR(__skew->next);
    }
    
    Xfirst = 0;
    //printf("Xtotal %d, Ytotal %d, XYtotal = %d, XXtotal %d\n",Xtotal,Ytotal,XYtotal,XXtotal);
    Ymead   = (float)Ytotal/(float)count;
    Xmead   = (float)Xtotal/(float)count;
    XYmead  = (float)XYtotal/(float)count;
    XXmead  = (float)XXtotal/(float)count;

    ///printf("sXmead %.3f, sYmead %.3f, sXYmead = %.3f, sXXmead %.3f\n",Xmead,Ymead,XYmead,XXmead);
    
    a = (XYmead - Xmead*Ymead)/(XXmead - Xmead*Xmead);
    return a;


}

//###########################################################################################################################
float calculateSkewTime(struct SkewData *__skew){
	float			tmp;
	static uint8_t		count;
	static float		min,max, total;
	static uint64_t		head_time;
	static struct timeval	head_offset;
	if(head_time == 0){
		head_time = __skew->sec_master;
		head_offset = __skew->offset;
		count = 1;
		min = 1000000;
		max = -1000000;
		total = 0;
		if(__skew->next!=NULL)return calculateSkewTime(__skew->next);
		return 0;
	}
	if(head_offset.tv_sec == __skew->offset.tv_sec){
		tmp = (float)(head_offset.tv_usec - __skew->offset.tv_usec) / (float)(__skew->sec_master - head_time );
	}
	else {
		if(head_offset.tv_usec < __skew->offset.tv_usec){
			tmp = (float)(1000000 + head_offset.tv_usec - __skew->offset.tv_usec) / (float)(__skew->sec_master - head_time );
		}
		else{
			tmp = (float)(head_offset.tv_usec - __skew->offset.tv_usec - 1000000) / (float)(__skew->sec_master - head_time );
		}
	}
	min = min > tmp ? tmp : min;
	max = max < tmp ? tmp : max;
	total += tmp;
	if(__skew->next != NULL){
		count++;
		return calculateSkewTime(__skew->next);
	}	
	head_time = 0;
	if(count>5) return (total - min - max)/(float)(count - 2);
	else return total/(float)count;
}

//###########################################################################################################################

struct timeval getLastOffset(struct SkewData *__skew){
	if(__skew->next != NULL){
		return getLastOffset(__skew->next);
	}
	else return __skew->offset;
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

//###########################################################################################################################
struct SkewData *deleteFirstNode(struct SkewData *__head){
	struct SkewData *secondNode = __head->next;
    memset(__head,'\0',sizeof(struct SkewData));
	free(__head);
	return secondNode;
}

//###########################################################################################################################
//###########################################################################################################################

int8_t isReadyToSendSkew(struct Coluns *__colun){
        if(colunsCount == 0)return NO;
        if(__colun !=NULL){
            if(__colun->newSkew == 0)return NO;
            return isReadyToSendSkew(__colun->next);
        }
        return YES;
}

//###########################################################################################################################

void timer(struct timeval *__time){
		struct timeval tp;
		gettimeofday(&tp,NULL);
		while(__time->tv_sec > tp.tv_sec)gettimeofday(&tp,NULL);
		while(__time->tv_usec > tp.tv_usec)gettimeofday(&tp,NULL);
}

//###########################################################################################################################

void changeTime(struct timeval *__time, int32_t sec, int32_t usec){
    __time->tv_sec +=   sec;
    __time->tv_usec +=  usec;
    if((int)__time->tv_usec >= 1000000){
        __time->tv_sec++;
        __time->tv_usec -=1000000;
    }
}
*/
