#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
// Just prints output for now 

const char* ARGP_PROGRAM_VERSION = 
	"timer-cli 1.0"; 

const char* ARGP_PROGRAM_BUG_ADDRESS = 
	"<joelfooxj@gmail.com>"; 

static char doc[] = 
	"A simple CLI timer"; 

static struct argp argp = {0,0,0,doc};

typedef struct timestamp {
	int seconds; 
	int minutes; 
	int hours;
} timestamp; 




int timer(int seconds){
	while(seconds >= 0){
		sleep(1);
		seconds--; 
		printf("\033c");
		printf("%dhr %dmin %dsec\n", seconds/3600, seconds/60, seconds); 
	}
	return 1; 
}

int stopwatch(){
	int total_seconds = 0;
	
	while (1){
		sleep(1);
		total_seconds++; 
		printf("\033c");
		printf("%dhr %dmin %dsec\n", total_seconds/3600, total_seconds/60, total_seconds); 
	}	 
	return 1;
}




int main(int argc, char* argv[]){ 
	argp_parse(&argp, argc, argv, 0, 0, 0);
	// stopwatch();
	exit(0);	

	
}

