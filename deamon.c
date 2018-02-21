#include "main.h"

int main (int argc, char *argv[]){
	FILE*	pidfile;
	int	pid;
	readAttr(argc,argv);
	switch(args.daemon){
		case 1:
			pid=fork();
			switch(pid){
				case 0:
					setsid();
					chdir("/");
					close(STDIN_FILENO);
					close(STDOUT_FILENO);
					close(STDERR_FILENO);
                    if(args.mode == CLIENT)startClient();
                    else startServer();
					_exit(0);
				break;
				case -1:
					printf("Error starting program\n");
				break;
				default:
					printf("The program was started in deamon mode with '%d'\n",pid);
					sleep(1);
				break;
			}
		break;
		case 0:
			printf("The program was started in normal mode with '%d'\n",getpid());
            if(args.mode == CLIENT)startClient();
            else startServer();
		break;

	}
return 0;
}
