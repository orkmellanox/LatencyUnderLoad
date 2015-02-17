#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"

int process_arg(char *argv[]);
void print_config(void);

struct config_t config = {
  	0,			/* Is server */
	"0",		/* Server's IP */
	5000,		/* Server's port */
	100,      	/* Message Size */
	100,		/* Message Rate Per Second */
	1,			/* period to run test */
	1,			/* number of connection */
	0,			/* warm up period */
	0,			/* TCP_NODELAY option */
	NON			/* Use extra connection for testing close, open or non. */ 
};

bool is_finished = false;

int  main(int argc, char *argv[])
{
  	int	test_result     = 1;
	runner_t* runner;

	if (argc < 11 || process_arg(argv)) {
      printf("usage: Incorrect parameter \n"
	  "%s <SERVER\\CLIENT> <server IP> <server Port> <Message Size> <Message Rate Per Second> <Test period for client side> <Number of connection> <WARMUP/NOWARMUP> <DELAY/NODELAY> <EXTRA_OPEN/EXTRA_CLOSE/NOEXTRA>\n", argv[0]);
      return -1;
    }
	
	print_config();
	
	if (config.server) {
		runner = new server_t();
	}
	else {
		runner = new client_t();
	}
	
	runner->run();
	
	runner->print_result();
	
	return 0;
}

/* 
* Trace entered argument to get Test configuration. 
* Return 0 if entered arguments are valid, -1 otherwise.
*/
int process_arg(char *argv[]) {

	if(strcmp(argv[1], "SERVER") == 0){
		config.server = 1;
	}
	else if(strcmp(argv[1], "CLIENT") == 0){
		config.server = 0;
	}
	else {
		printf("unknown application type %s\n", argv[1]);
		return -1;
	}
	
	strcpy(config.server_ip, argv[2]);
			
	config.server_port = atoi(argv[3]);

	config.message_size = atoi(argv[4]);

	config.mps = atoi(argv[5]);

	config.period = atoi(argv[6]);

	config.number_connection = atoi(argv[7]);

	if(strcmp(argv[8], "WARMUP") == 0){
		config.warm = WARM_UP_PERIOD;
	}
	else if(strcmp(argv[8], "NOWARMUP") == 0){
		config.warm = 0;
	}
	else {
		printf("unknown parameter %s\n", argv[8]);
		return -1;
	}
	
	if(strcmp(argv[9], "DELAY") == 0){
		config.tcp_nodelay = 0;
	}
	else if(strcmp(argv[9], "NODELAY") == 0){
		config.tcp_nodelay = 1;
	}
	else {
		printf("unknown parameter %s\n", argv[9]);
		return -1;
	}
	
	if(strcmp(argv[10], "EXTRA_OPEN") == 0){
		config.extra_op = OPEN;
	}
	else if(strcmp(argv[10], "EXTRA_CLOSE") == 0){
		config.extra_op = CLOSE;
	}
	else if(strcmp(argv[10], "NOEXTRA") == 0){
		config.extra_op = NON;
	}
	else {
		printf("unknown parameter %s\n", argv[10]);
		return -1;
	}
	
	return 0;
}

/* 
* Print configuration that will be used in test.
*/
void print_config(void)
{
  	printf("-----------------------------------------\n");
	printf("Is Server:                  %s\n", config.server ? "YES" : "NO");
	printf("Server IP:                  %s\n", config.server_ip);
	printf("Server Port:                %d\n", config.server_port);
	printf("Message Size:               %d\n", config.message_size);
	printf("Message Rate Per Second:    %d\n", config.mps);
	printf("period to run test:         %d\n", config.period);
	printf("Number of connection:       %d\n", config.number_connection);
	printf("With WARM UP Period:        %s\n", config.warm ? "YES" : "NO");
	printf("With TCP_NODELAY option:    %s\n", config.tcp_nodelay ? "YES" : "NO");
	printf("With EXTRA connections:     %s\n", (config.extra_op != NON) ? "YES" : "NO");
	printf("-----------------------------------------\n");
}





