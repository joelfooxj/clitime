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
// TODO: Major bug -> there's lag from post processing

// Need to redesign this to use either multi-threading or to use 
// the processor clock

// -> Stopwatch: simply mark the start time and then print the current offset from that time 

// -> Timer: Mark the start time and subtract current offset from time period

// -> Pause: set start time to now, and offset to 0


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
	{"resolution", 'r', "u/m/c/s", 0, "Sets the resolution. u(microsecond), m(millisecond), c(centisecond), s(second)."},	
	{"stopwatch", 's', 0, 0, "Starts a stopwatch."},	
	{"timer", 't', "XhYmZs", 0, 
		"Starts a timer. Time components (Xh, Ym, Zs) can be of any combination, \
but must be in order. X,Y,Z can be any integer above 0. \
Eg. 2m30s=2minutes 30seconds, 1h20m=1hour 20minutes, 10s=10seconds."}, 
	{0},	

};

const char* commands_desc = "Start/Stop(space), Reset(r), Exit(e)\n";

void time_printout(unsigned long mseconds, int status){ 
	unsigned long second_multiplier = 1e6/micro_multiplier;
	printf("\033c");
	printf("%luhr %lumin %lu.%lusec\n", 
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
	unsigned long original_mseconds = (isIncrement) ? 0 : (1e6/micro_multiplier)*seconds;
	unsigned long mseconds = original_mseconds;
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
	const char* chunkReps[] = {"hours", "minutes", "seconds"};
	int nums[3] = {0,0,0}; 

	int foundTokens = 0;
	for(int i = 0; i < 3; i++){ 
		char* pos = strchr(string_check, chunks[i]); 
		if (pos != NULL){
			int time_num = atoi(strtok(foundTokens <= 0 ? arg:NULL, "hms")); 
			if (time_num <= 0){
				printf("Invalid time value for %s\n", chunkReps[i]);
				exit(1);
			} else { 
				nums[i] = time_num;
			}
			foundTokens++;
		}
	}	
	return (nums[0]*3600)+(nums[1]*60)+(nums[2]);
}

int set_resolution(char* arg){ 
	 
	if (strlen(arg) != 1){ 
		printf("Resolution argument length too long.");   
		exit(1);
	}
	
	char resolution = arg[0]; 
	switch(resolution){
		case 'u':
			micro_multiplier = 1;
			break;
		case 'm':
			micro_multiplier = 1e3;
			break;
		case 's':
			micro_multiplier = 1e6;
			break;
		case 'c':
			micro_multiplier = 1e3*100;
			break;
		default: 
			printf("Invalid resolution entered.\n");   
			exit(1);
	}	
	return 0;
}

typedef struct time_arg{
	int isIncrement; 
	int seconds;
} time_arg;

time_arg t_args; 

int number_selected = 0;

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'r':
	   set_resolution(arg); 	
       break;
    case 's':
		number_selected++;
	  t_args.isIncrement = 1; 
	  t_args.seconds = 0;
      break;
    case 't':
		number_selected++;
	  t_args.isIncrement = 0; 
	  t_args.seconds = parse_time(arg);
      break;
    case ARGP_KEY_END:
	    break;
	case ARGP_KEY_ERROR: 
		printf("Error parsing\n");
		break;
	case ARGP_KEY_FINI: 
		printf("finished parsing\n");
		break;
	
	case ARGP_KEY_SUCCESS: 
		// 1. Check if only either t or s has been selected 
		if (number_selected != 1){
			printf("Please select either timer or stopwatch functionality\n"); 
			exit(1);
		}
		time_counter(t_args.isIncrement, t_args.seconds);
		break;
	}
  return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int main(int argc, char* argv[]){ 
	return argp_parse(&argp, argc, argv, 0, 0, 0);
}

