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
    printf("Thread offset was started\n");
    
	struct slave_t	*slave  = (struct slave_t *)__slave;
	uint16_t		RTTcount;	
	int8_t			success_count, bestResult, offsetDir, running = 1, offset_fails,res;
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
        gettimeofday(&currTime,NULL);
		if((res = getOffsetOfServer(slave,&sec,&skew_clock)) <=0){
            if(offset_fails>10)goto rtt_start;
            offset_fails++;
            continue;
        }
        
	/*	
        colun->status = STATUS_NONE;
        if(colun->skew != NULL){
			if(success_count == SKEW_COUNT){
				colun->skew = deleteFirstNode(colun->skew);
				success_count--;

			}
			currentNode = addNewNode(currentNode,&sec,&skew_clock );
			skewTMP = calculateSkewTime(colun->skew);
            colun->actualSkewLR = calculateSkewLR(colun->skew);
            colun->actualSkew = skewTMP;
            colun->newSkew = YES;
			//printf("%p : SkewMead = %.3f\t, SkewLR %.3f\n",colun, colun->actualSkew, colun->actualSkewLR*-1);
			printf("%p : SkewLR %.3f\n",colun, colun->actualSkewLR*-1);
			success_count++;
		}
		else{
			colun->skew = addNewNode(colun->skew,&sec, &skew_clock);
			currentNode = colun->skew;
			success_count++;
		}
        */
    gettimeofday(&currTime,NULL);
    sleep(DELAY);
	}

}

/*
//######################################################################################################################################################
void *action_thrd_func(){
    printf("\tAction thread has been started\n");
    playingStatus = STOP_S;	
	int16_t		byte, i,y;
	unsigned char	buffer[BUFFSIZE];
	int32_t		fd_wav, count, bytes;
    uint16_t    tmp;
    struct Coluns   *currColun;
	
	while(isExit == 0 ){
        recvfrom(sock_unix,buffer,100,0,NULL,NULL );
		switch(buffer[0]){
            case VOLUME_S:
                    buff_play[0] = VOLUME;
                    buff_play[1] = buffer[1];
                    currColun = colunsHead;
                    while(currColun != NULL){
                        sendto(sock,buff_play,2,0,(struct sockaddr*)&currColun->sock,sizeof(currColun->sock));
                        currColun = currColun->next;
                    }
                break;
			case PLAY_S:
				if(colunsCount == 0){ 
                    printf("Don't have any colun for play\n");
                    continue;
                }
                if(playingStatus == PLAY_S || playingStatus == PAUSE_S){
                    stopPlay();
                }
                musicToPlay = (uint16_t)buffer[1];
				music = getMusicFileById(fileList->head, musicToPlay);
                if(music == NULL){
                    music = fileList->head;
                    musicToPlay = fileList->head->id;
                }
				switch(music->format){
					case MP3:
						samples = parseMP3File( &wavHead, music);
						fileSize = music->sizeTotal/music->channels;	
					break;
					case WAV:
                        printf("WAV is not supported\n");
						//samples = parseWAVFile( &wavHead, fileName);
						//fileSize = (wavHead.chunkSize-36)/2;	
					break;
					default:
                        printf("Format = %d, is not supported\n",music->format);
				}
				if(samples == NULL){
					printf("Samples = NULL\n");
					break;
				}
			 	rightChannel  = (struct monoSamples_t *)malloc(fileSize);
				leftChannel  = (struct monoSamples_t *)malloc(fileSize);
                getLeftRightChannels(samples, rightChannel, leftChannel, music);
				
				playingStatus = PLAY_S;
                currColun = colunsHead;
                while(currColun != NULL){
                     pthread_create(&currColun->thrdData,NULL,&thrd_func_for_send_data,currColun);
                     currColun = currColun->next;
                }
                pthread_create(&thrd_for_start_play,NULL,&thrd_func_start_play_when_is_ready,NULL);
				break;
			case STOP_S:
				if(playingStatus == PLAY_S){
					stopPlay();
				}
				break;
			case PAUSE_S:
				//if(playingStatus == PLAY_S){
                  //  playingStatus = PAUSE_S;
				    pausePlay();
                //}else if(playingStatus == PAUSE_S) {
                  //  playingStatus = PLAY_S;
				    //pausePlay();
                //}
				break;
            case SET_S:
                setPlay((uint16_t)(buffer[2]<<8 | buffer[1]));
            break;
			default:
				printf("Action thread switch default\n");

		}
	}
}

//######################################################################################################################################################

*/
