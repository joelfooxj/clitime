#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
#include<string.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<stdbool.h>


// TODO: Improve time parser for out-of-order components
// TODO: Improve error checking for time parser
// TODO: Should probably assign timer/UI to a separate thread.
// -> Put the time functions running on another thread. I can't use sleep to increment the mseconds... 
// timer thread could use poll... but I'd have to setup some kinda of shared buffer as a fd for it poll... 
// timer thread could use a mutex timeout thingy

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
		"Starts a timer. Time components (Xh, Ym, Zs) can be of any combination,  \
but must be in order. \
Eg. 2m30s=2minutes 30seconds, 1h20m=1hour 20minutes, 10s=10seconds."}, 
	{0},	

};

const char* commands_desc = "Start/Stop(space), Reset(r), Exit(e)\n";

void time_printout(int mseconds, int status){ 
	printf("\033c");
	printf("%dhr %dmin %dsec %dmsec\n", 
		(mseconds/1000)/3600, 
		(mseconds/60000)%60, 
		(mseconds/1000)%60, 
		mseconds%1000); 
	printf("%s",commands_desc); 
	if (status == 1){
		printf("Running.\n");  
	} else { 
		printf("Stopped.\n");
	}
}

int timer(int seconds){
	int original_mseconds = 1000*seconds;
	int mseconds = 1000*seconds;
	int runningFlag = 1;
	while(1){
		usleep(1e3);
		char commandStr;
		if (kbhit()){
			commandStr = getchar();
			switch(commandStr){
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
		}		
		time_printout(mseconds, runningFlag);
		if (mseconds > 0 && runningFlag == 1) mseconds--;
	}
	return 1; 
}

int stopwatch(){
	int mseconds = 0;
	int runningFlag = 1; 	
	while (1){
		// poll every 1msec
		usleep(1e3);
		char commandStr; 
		if (kbhit()){
			commandStr = getchar();
			switch(commandStr){
				case ' ': 
					runningFlag = (runningFlag+1) % 2;
					break;
				case 'e': 
					exit(0);
				case 'r': 
					mseconds = 0;
					break; 
				default: 
					// Invalid commands are ignored.
					break;
			}	
		}		
		time_printout(mseconds, runningFlag);
		if (runningFlag == 1) mseconds++;
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

