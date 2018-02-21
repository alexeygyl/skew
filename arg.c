#include "main.h"

void help(void);

void readAttr(int _argc, char ** _argv){
	int rez;
	args.port = 9999;	//Default
	args.daemon = NO;	//Default is not deamon
    args.mode = CLIENT;
	while((rez=getopt(_argc,_argv,ARG))!=-1){
		switch(rez){
			case 'p':	
				sscanf(optarg,"%" SCNd16,&args.port);
			break;
			case 'D':
				args.daemon = YES;
			break;
            case 'S':
                args.mode = SERVER;
            break;
            case 'C':
                args.mode = CLIENT;
            break;
			case 'h':
				help();
				exit(0);
			break;
			default:
			break;

		}

	}

}

void help(void){
	printf("Usage:\n");
	printf("  binary [OPTIONS]\n");
	printf("\n\n");
	printf("[OPTIONS]\n");
	printf("  -D <mode>\t\texecute program in deamon mode, no Deamon is default\n");
	printf("  -p <port>\t\tport of snmp agent, 9999 is default\n");
	printf("  \n");
	printf("Exemple: binary -D -p 6666\n");

}
