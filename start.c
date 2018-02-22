#include "main.h"

void startServer(){
    printf("SERVER MODE \n");
	createUDPSocket(&sock, &args.port);
    source_len = sizeof(source);
    struct timeval tstamp;
    while(1){
        memset(rxbuff,'\0',BUFFSIZE);
        bytes_r = recvfrom(sock,rxbuff,BUFFSIZE,0,(struct sockaddr*)&source, &source_len);
        switch(rxbuff[0]){
            case BROADCAST:
                txbuff[0] = MASTER;
                sendto(sock,txbuff,1,0,(struct sockaddr*)&source, sizeof(source));
            break;
            case OFFSET_REQ:
                rxbuff[0] = OFFSET_RES;
                gettimeofday(&tstamp,NULL);
                memcpy(&rxbuff[5],&tstamp.tv_sec,4);
                memcpy(&rxbuff[9],&tstamp.tv_usec,4);
                //printf("Size %d,  %d.%d\n",sizeof(struct timeval), tstamp.tv_sec, tstamp.tv_usec);
                sendto(sock,rxbuff,bytes_r + 8,0,(struct sockaddr*)&source, sizeof(source));
            break;
            case RTT_REQ:
                rxbuff[0] = RTT_RES;
                sendto(sock,rxbuff,bytes_r,0,(struct sockaddr*)&source, sizeof(source));
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
    slave.rtt = NULL;
    slave.skew = NULL;
    slave.actualSkew = 0;
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
				memcpy(&slave.packet.seq, &rxbuff[1],4);
				if(slave.packet.seq != slave.expected_p)continue;
				memcpy(&slave.packet.time.tv_sec,&rxbuff[5],4);
				memcpy(&slave.packet.time.tv_usec,&rxbuff[9],4);
                //printf("OFFSET_RES %d  = %d, %d.%d\n", slave.packet.seq, slave.expected_p, slave.packet.time.tv_sec, slave.packet.time.tv_usec);
				slave.packet.received = TRUE;
			break;
			case RTT_RES:
				memcpy(&slave.packet.seq, &rxbuff[1],4);
				if(slave.packet.seq != slave.expected_p)continue;
				slave.packet.received = TRUE;
			break;
            default:
                printf("Unknow tag '%d'\n", rxbuff[0]);

		}
	}
}

