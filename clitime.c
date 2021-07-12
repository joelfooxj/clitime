#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
#include<string.h>
#include<poll.h>

// TODO: Improve the response time of commands
// TODO: Use enter as a start/stop toggle? 
// TODO: Improve time parser for out-of-order components



const char* ARGP_PROGRAM_VERSION = 
	"timer-cli 1.0"; 

const char* ARGP_PROGRAM_BUG_ADDRESS = 
	"<joelfooxj@gmail.com>"; 

static char doc[] = 
	"A simple CLI timer/stopwatch"; 

static char args_doc[] = "stopwatch timer"; 

static struct argp_option options[] = {
	{"stopwatch", 's', 0, 0, "Starts a stopwatch."},	
	{"timer", 't', "XhYmZs", 0, 
		"Starts a timer. Time components (Xh, Ym, Zs) can be of any combination,  \
but must be in order. \
Eg. 2m30s=2minutes 30seconds, 1h20m=1hour 20minutes, 10s=10seconds."}, 
	{0},	

};

const char* commands_desc = "Start/Stop(s), Reset(r), Exit(e)\n";

int command_parser(char* command){
	// Accepts the following chars: p,r,c,e
	if (strlen(command) > 1){
		printf("%s is invalid.\n", command);
		return -1;
	}

	char com = command[0];
	switch(com){ 
		case 's': 
			// start/stop
			return 1; 
			break; 
		case 'e': 
			// exit
			return 2; 
			break; 
		case 'r': 
			// reset 
			return 3; 
			break;
		default: 
			printf("%s is invalid.\n", command);
			return -1;
			break;
	}

	return -1;
}

int poll_sec(char* command){
	struct pollfd fds[1]; 
	int timeout = 0; 
	fds[0].fd = STDIN_FILENO; 
	fds[0].events = 0; 
	fds[0].events = fds[0].events | POLLIN;
	
	int result = poll(fds, 1, timeout);
	if (result == 1){ 
		// extract command from stdin 
		// fgets(command, 1, stdin);
		scanf("%1s", command);
		fflush(stdin);
		return 1;
	} else return 0; 

	// should not get here.
	return -1;
}


int timer(int seconds){
	int original_seconds = seconds;
	int runningFlag = 1;
	while(1){
		// poll for 1 second here 
		sleep(1);
		char commandStr[10];
		int response = poll_sec(commandStr);
		if (response == 1){
			int command = command_parser(commandStr);
			switch(command){
				case 1: 
					// toggle runningFlag
					runningFlag = (runningFlag+1) % 2;
					break;
				case 2: 
					exit(0);
				case 3: 
					seconds = original_seconds;
					break; 
				default: 
					// Invalid commands are ignored.
					break;
			}	
		}		

		printf("\033c");
		printf("%dhr %dmin %dsec\n", seconds/3600, (seconds/60)%60, seconds%60); 
		printf("%s",commands_desc); 
		if (seconds > 0 && runningFlag == 1){
			seconds--;
			printf("Running.\n");  
		} else { 
			printf("Stopped.\n");
		}
			
	}
	return 1; 
}

int stopwatch(){
	int seconds = 0;
	int runningFlag = 1; 	
	while (1){
	    // poll for 1 second here 
		sleep(1);
		char commandStr[100];
		int response = poll_sec(commandStr);
		if (response == 1){
			int command = command_parser(commandStr);
			switch(command){
				case 1: 
					runningFlag = (runningFlag+1) % 2;
					break;
				case 2: 
					exit(0);
				case 3: 
					seconds = 0;
					break; 
				default: 
					// Invalid commands are ignored.
					break;
			}	
		}		

		printf("\033c");
		printf("%dhr %dmin %dsec\n", seconds/3600, (seconds/60)%60, seconds%60); 
		printf("%s",commands_desc); 
		if (runningFlag == 1){
			seconds++; 
			printf("Running.\n");
		} else { 
			printf("Stopped.\n");
		}
	}	 
	return 1;
}

int parse_time(char* arg){
	// string should be in format XhYmZs
	// Any combination of Xh, Ym, Zs, in that order

	char* string_check = (char*)malloc(strlen(arg)+1);
	strncpy(string_check,arg, strlen(arg)+1);

	const char chunks[] = {'h','m','s'};
	int nums[3] = {0,0,0}; 

	int foundTokens = 0;
	for(int i = 0; i < 3; i++){ 
		char* pos = strchr(string_check, chunks[i]); 
		if (pos != NULL){
			nums[i] = atoi(strtok(foundTokens <= 0 ? arg:NULL, "hms")); 
			foundTokens++;
		}
	}	
	free(string_check);
	return (nums[0]*3600)+(nums[1]*60)+(nums[2]);
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 's':
      printf("stopwatch started...\n");
	  stopwatch(); 
      break;
    case 't':
      printf("timer started...\n");
	  int seconds =	parse_time(arg); 
	  timer(seconds);
      break;
    }
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]){ 
	argp_parse(&argp, argc, argv, 0, 0, 0);
	exit(0);	

	
}

