#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
#include<string.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<stdbool.h>
#include<poll.h>

// TODO: Improve time parser for out-of-order components
// TODO: Improve error checking for time parser
// TODO: Improve arg parser error checking
// TODO: Add the following second resolutions: 
// 1. centiseconds
// 2. milliseconds
// 3. microseconds

int micro_multiplier = 1e3*100;

int kbhit(void) {
    static bool initflag = false;
    static const int STDIN = 0;

    if (!initflag) {
        // Use termios to turn off line buffering
        struct termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initflag = true;
    }

    int nbbytes;
    ioctl(STDIN, FIONREAD, &nbbytes);  // 0 is STDIN
    return nbbytes;
}
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
		"Starts a timer. Time components (Xh, Ym, Zs) can be of any combination, \
but must be in order. X,Y,Z can be any integer above 0. \
Eg. 2m30s=2minutes 30seconds, 1h20m=1hour 20minutes, 10s=10seconds."}, 
	{0},	

};

const char* commands_desc = "Start/Stop(space), Reset(r), Exit(e)\n";

void time_printout(int mseconds, int status){ 
	int second_multiplier = 1e6/micro_multiplier;
	printf("\033c");
	printf("%dhr %dmin %d.%dsec\n", 
		(mseconds/second_multiplier)/3600, 
		(mseconds/(60*second_multiplier))%60, 
		(mseconds/second_multiplier)%60, 
		mseconds%second_multiplier); 
	printf("%s",commands_desc); 
	if (status == 1){
		printf("Running.\n");  
	} else { 
		printf("Stopped.\n");
	}
}

int time_counter(int isIncrement, int seconds){
	int original_mseconds = (isIncrement) ? 0 : (1e6/micro_multiplier)*seconds;
	int mseconds = original_mseconds;
	int runningFlag = 1;
	int pollRet = 0;
	struct pollfd fds[1]; 
	fds[0].fd = STDIN_FILENO;
	fds[0].events = POLLIN; 
	while(1){
		usleep(micro_multiplier);
		time_printout(mseconds, runningFlag);
		if (!runningFlag){
			pollRet = poll(fds, 1, -1);
		}
		if (kbhit()){
			switch(getchar()){
				case ' ': 
					// toggle runningFlag
					runningFlag = (runningFlag+1) % 2;
					break;
				case 'e': 
					exit(0);
				case 'r': 
					mseconds = original_mseconds;
					break; 
				default: 
					// Invalid commands are ignored.
					break;
			}	
			time_printout(mseconds, runningFlag);
		}		
		if (runningFlag){
			if (isIncrement) mseconds++; 
			else mseconds --;
		}
		if (mseconds <= 0) runningFlag = 0;
	}
	return 1; 
}

int parse_time(char* arg){
	// string should be in format XhYmZs
	// Any combination of Xh, Ym, Zs, in that order

	char string_check[strlen(arg)+1];
	strncpy(string_check, arg, strlen(arg)+1);

	const char chunks[] = {'h','m','s'};
	int nums[3] = {0,0,0}; 

	int foundTokens = 0;
	for(int i = 0; i < 3; i++){ 
		char* pos = strchr(string_check, chunks[i]); 
		if (pos != NULL){
			int time_num = atoi(strtok(foundTokens <= 0 ? arg:NULL, "hms")); 
			if (time_num <= 0){
				printf("Invalid time value for %c\n", chunks[i]);
				exit(1);
			} else { 
				nums[i] = time_num;
			}
			foundTokens++;
		}
	}	
	return (nums[0]*3600)+(nums[1]*60)+(nums[2]);
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 's':
      printf("stopwatch started...\n");
	  time_counter(1,0); 
      break;
    case 't':
      printf("timer started...\n");
	  time_counter(0,parse_time(arg));
      break;
    case ARGP_KEY_END:
	  // At least one argument expected.
	  if(state->arg_num < 1)
		argp_usage(state);
	  break;
	}
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]){ 
	argp_parse(&argp, argc, argv, 0, 0, 0);
	exit(0);	

	
}

