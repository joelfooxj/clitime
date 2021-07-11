#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
// Just prints output for now 
// TODO: Add ability to pause 

const char* ARGP_PROGRAM_VERSION = 
	"timer-cli 1.0"; 

const char* ARGP_PROGRAM_BUG_ADDRESS = 
	"<joelfooxj@gmail.com>"; 

static char doc[] = 
	"A simple CLI timer/stopwatch"; 

static char args_doc[] = "stopwatch timer"; 

static struct argp_option options[] = {
	{"stopwatch", 's', 0, 0, "Start a stopwatch."},	
	{"timer", 't', "SECONDS", 0, "Start a timer."}, // requires a string like 2h3m4s	
	{0},	

};

int timer(int seconds){
	while(1){
		sleep(1);
		if (seconds > 0){
			seconds--; 
		}
		printf("\033c");
		printf("%dhr %dmin %dsec\n", seconds/3600, seconds/60, seconds%60); 
	}
	return 1; 
}

int stopwatch(){
	int total_seconds = 0;
	
	while (1){
		sleep(1);
		total_seconds++; 
		printf("\033c");
		printf("%dhr %dmin %dsec\n", total_seconds/3600, total_seconds/60, total_seconds%60); 
	}	 
	return 1;
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
	  int seconds = atoi(arg); 
	  printf("%d\n", seconds); 
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

