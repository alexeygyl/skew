#include "main.h"

void startServer(){
    printf("SERVER is started \n");
	createUDPSocket(&sock, &args.port);
    source_len = sizeof(source);
    while(1){
        memset(rxbuff,'\0',BUFFSIZE);
        bytes_r = recvfrom(sock,rxbuff,BUFFSIZE,0,(struct sockaddr*)&source, &source_len);
        switch(rxbuff[0]){
            case BROADCAST:
                txbuff[0] = MASTER;
                sendto(sock,txbuff,1,0,(struct sockaddr*)&source, sizeof(source));
            break;
            default:
                printf("Unknow tag '%d\n'", rxbuff[0]);
		}

    }
}

void startClient(){
    printf("CLIENT is started \n");
	createUDPSocket(&sock, &args.port);
    initBroadcastMessage(&sock);
	pthread_create(&thrd_server_discover,NULL,&thrd_func_server_discover,NULL);
    source_len = sizeof(source);
	while(1){
        memset(rxbuff,'\0',BUFFSIZE);
        bytes_r = recvfrom(sock,rxbuff,BUFFSIZE,0,(struct sockaddr*)&source, &source_len);
		switch(rxbuff[0]){
            case BROADCAST:
                //printf("\tBROADCAST....\n");
            break;
            case MASTER:
                master = source;
                printf("Master was discovered\n");
            break;
            /*
            case INIT_REQUEST:
                if(colunsCount == 0){
                    colunsLast = colunsHead = addNewColun(colunsHead,&client,buff[1]);
                    pthread_create(&colunsLast->thrdMain,NULL,&thrd_func_for_offset,colunsLast);
                    //printTree(colunsHead);
                    //printf("0:New colun %p, count %d\n",colunsLast, colunsCount);
                    break;
                }
                colun = getColunByAddr(colunsHead,&client);
                if(colun != NULL){
                    gettimeofday(&colun->online,NULL);
                }
                else {
                    colunsLast = addNewColun(colunsLast,&client, buff[1]);
                    pthread_create(&colunsLast->thrdMain,NULL,&thrd_func_for_offset,colunsLast);
                    //printTree(colunsHead);
                    //printf("1:New colun %p, count %d\n",colunsLast, colunsCount);
                }
			break;
		
            case OFFSET_RESPONCE:
                colun = getColunByAddr(colunsHead,&client);
				if(colun !=NULL){
					memcpy(&tmp, &buff[1],4);
					if(colun->expected_p != tmp)continue;
					memcpy(&colun->packet.time.tv_sec,&buff[5],4);
					memcpy(&colun->packet.time.tv_usec,&buff[9],4);
					colun->packet.seq = tmp;
					colun->packet.status = 1;
				}
			break;
			case RTT_RESPONCE:
                colun = getColunByAddr(colunsHead,&client);
				if(colun != NULL){
					memcpy(&tmp, &buff[1],4);
					if(colun->expected_p != tmp)continue;
					colun->packet.seq = tmp;
					colun->packet.status = YES;

				}
			break;
            */
            default:
                printf("Unknow tag '%d\n'", rxbuff[0]);

		}
	}
}

