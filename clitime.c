#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
#include<string.h>
// Just prints output for now 
// TODO: Add ability to pause (p) 
// TODO: Add reset (r)
// TODO: Add continue (c) 
// TODO: Add exit (e)
// Either write a global interrupt handler, or a time-limited input prompt...
// global handler seems to complicated... 
// for now, just a simple scan..... 
// wait for user input for 1 sec, check if stdin has anything in the buffer 
// use poll to check for any input in stdin. 

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

int timer(int seconds){
	while(1){
		sleep(1);
		if (seconds > 0){
			seconds--; 
		}
		printf("\033c");
		printf("%dhr %dmin %dsec\n", seconds/3600, (seconds/60)%60, seconds%60); 
	}
	return 1; 
}

int stopwatch(){
	int total_seconds = 0;
	
	while (1){
		sleep(1);
		total_seconds++; 
		printf("\033c");
		printf("%dhr %dmin %dsec\n", total_seconds/3600, (total_seconds/60)%60, total_seconds%60); 
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
	  // for now just convert arg into seconds 
	  int seconds =	parse_time(arg); 
	  // int seconds = atoi(arg); 
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

