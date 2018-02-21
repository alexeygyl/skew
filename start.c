#include "main.h"

void startServer(){
    printf("SERVER MODE \n");
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
            case OFFSET_REQ:
                printf("OFFSET_REQ\n");
            break;
            case RTT_REQ:
                rxbuff[0] = RTT_RES;
                sendto(sock,rxbuff,bytes_r,0,(struct sockaddr*)&source, sizeof(source));
                printf("RTT_REQ\n");
            break;
            default:
                printf("Unknow tag '%d'\n", rxbuff[0]);
		}

    }
}

void startClient(){
    printf("CLIENT MODE\n");
    createUDPSocket(&sock, &args.port);
    initBroadcastMessage(&sock);
	pthread_create(&thrd_server_discover,NULL,&thrd_func_server_discover,NULL);
    source_len = sizeof(source);
    struct slave_t  slave;
	while(1){
        memset(rxbuff,'\0',BUFFSIZE);
        bytes_r = recvfrom(sock,rxbuff,BUFFSIZE,0,(struct sockaddr*)&source, &source_len);
		switch(rxbuff[0]){
            case BROADCAST:
                //printf("\tBROADCAST....\n");
            break;
            case MASTER:
                master = source;
                isMasterDiscovered = TRUE;
	            pthread_create(&thrd_offset,NULL,&thrd_func_offset,&slave);
            break;
            case OFFSET_RES:
                printf("OFFSET_RES\n");
                /*
                colun = getColunByAddr(colunsHead,&client);
				if(colun !=NULL){
					memcpy(&tmp, &buff[1],4);
					if(colun->expected_p != tmp)continue;
					memcpy(&colun->packet.time.tv_sec,&buff[5],4);
					memcpy(&colun->packet.time.tv_usec,&buff[9],4);
					colun->packet.seq = tmp;
					colun->packet.status = 1;
				}*/
			break;
			case RTT_RES:
				memcpy(&slave.packet.seq, &rxbuff[1],4);
				if(slave.packet.seq = slave.expected_p)continue;
                printf("RTT_RES \n", slave.packet.seq);
				slave.packet.received = TRUE;
			break;
            default:
                printf("Unknow tag '%d'\n", rxbuff[0]);

		}
	}
}

