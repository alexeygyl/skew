#include "main.h"


void *thrd_func_server_discover(){
    printf("\tServer discover thread has been started\n"); 
    unsigned char msg[1];
    msg[0] = BROADCAST;
    while(1){
        broadcastServerDiscover.sin_addr.s_addr = inet_addr("192.168.1.218");
        sendto(sock,msg,1,0,(struct sockaddr*)&broadcastServerDiscover,sizeof(broadcastServerDiscover));
        sleep(BROADCAST_DELAY);
    }
    
}


/*
void *thrd_func_for_offset(void *__colun){
    
	struct Coluns	*colun  = (struct Coluns *)__colun;
	uint16_t		RTTcount;	
	int8_t			success_count, bestResult, offsetDir, running = 1, offset_fails,res;
	uint8_t			skew_position = 0;
	uint32_t		total_rtt = 0;
	uint64_t		sec;
	float			clockSkew, skewTMP;
	struct timeval		skew_clock, currTime;
	struct SkewData		*currentNode = NULL;
	d1 = d2 = 0;
    	//-----------------------------------------------------------------------------------
	printf("New colun %p, channel %d\n",colun, colun->channel);

    //------------------Get delays Range---------------------------------------------------
    RTTcount = RTT_REQUEST_COUNT;
rtt_start:;
    if(currTime.tv_sec - colun->online.tv_sec > COLUNS_OFFLINE_TIMEOUT) goto close_colun;
    colun->minRTT = 0;
    colun->maxRTT = RTT_REQUEST_TIMEOUT;
    offset_fails=0;
    if((RTTcount = getRTT(colun,&colun->minRTT,&colun->maxRTT)) == 0) {
		printf("RTT get error\n");
        goto close_colun;
	}
	getRTTInterval(colun->rttGausa,&colun->minRTT,&colun->maxRTT,RTTcount);
	free(colun->rttGausa);
	exit(0);
    printf("%p: Range  %d-%d\n",colun,colun->minRTT,colun->maxRTT);
	printf("--------------------------------------------------\n");

    //--------------------------------------------------------------------------------	
	while(currTime.tv_sec - colun->online.tv_sec<COLUNS_OFFLINE_TIMEOUT){
        gettimeofday(&currTime,NULL);
        if(currTime.tv_sec-colun->online.tv_sec>COLUNS_STOP_SERVICE_TIMEOUT){
            printf("Colun '%p' don't responce...\n", colun);
            sleep(1);
            continue;
        }
		if(colun->status == STATUS_NONE  || colun->status == STATUS_OFFSET){
            colun->status = STATUS_OFFSET;
			if((res = getOffsetFromColun(colun,&sec,&skew_clock)) == -1){
                if(offset_fails>10)goto rtt_start;
                offset_fails++;
                continue;
            }else if(res == -2){
                continue;
            }
		}
		else {
            usleep(100000);
			continue;
		}
        
		
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
    if(currTime.tv_sec-colun->online.tv_sec<COLUNS_STOP_SERVICE_TIMEOUT)sleep(DELAY);
    gettimeofday(&currTime,NULL);
	}
close_colun:;
    if(colun->pref != NULL){
        colun->pref->next = colun->next;
    }else{ 
        //printf("New head %p\n",colun->next);
        colunsHead = colun->next;
    }
    if(colun->next !=NULL){
        colun->next->pref = colun->pref;
    }else {
        //printf("New last %p\n",colun->pref);
        colunsLast = colun->pref;
    }
    memset(colun,'\0',sizeof(struct Coluns));
    free(colun);
    //printTree(colunsHead);
    colunsCount--;
    printf("Colun '%p' is offline, close thread and realise all resoureces...\n",colun);

}


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
