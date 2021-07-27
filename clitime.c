#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h>
#include<argp.h>
#include<time.h>
#include<string.h>
#include<termios.h>
#include<sys/ioctl.h>
#include<stdbool.h>
#include<pthread.h>
#include<errno.h>
#include<poll.h>

// TODO: Improve time parser for out-of-order components
// TODO: Improve error checking for time parser
// TODO: Improve arg parser error checking
// TODO: BUG -> sometimes r and e stop working
#define DO_NOTHING 2; 

pthread_mutex_t run_lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t run_cond; 
int command_mtx = DO_NOTHING;

typedef struct time_args { 
	int isIncrement; 
	int seconds;
} time_args;

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

// command_mtx: 
// 1/0 toggle 
// -1 exit 
// -2 reset
void* time_counter(void* args){
	int original_mseconds = ((time_args*)args)->isIncrement ? 0:1000*((time_args*)args)->isIncrement;
	int mseconds = 1000*((time_args*)args)->seconds; 
	int running = 1;
	while(1){
		usleep(1e3);
		pthread_mutex_lock(&run_lock); 
		
		if (!running){ 
			time_printout(mseconds, running);
			pthread_cond_wait(&run_cond, &run_lock);
		} else { 
			if (((time_args*)args)->isIncrement) mseconds++;
			else mseconds--;
			time_printout(mseconds, running);
		}
		
		if (mseconds <= 0) running = 0;
	
		switch(command_mtx){
			case 1: 
				// toggle
				running = (running+1)%2; 
				break;
			case 0: 
				// exit
				pthread_mutex_unlock(&run_lock);
				return (void*)0;
				break;
			case -1: 
				mseconds = original_mseconds;
				time_printout(mseconds, running);
				break; 
			default: 
				// Invalid commands are ignored.
				// printf("Invalid command:%d\n", command_mtx); 
				break;
		}	
		command_mtx = DO_NOTHING;
		pthread_mutex_unlock(&run_lock);
	}
	return (void*)1; 
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

int UI_loop(){
	// Changes the shared var command_mtx
	int running = 1;
	struct pollfd fds[1]; 
	fds[0].fd = STDIN_FILENO;  
	fds[0].events = POLLIN;
	while(running){
		int ret = poll(fds, 1, -1); 	
		if (kbhit()){
			pthread_mutex_lock(&run_lock);
			switch(getchar()){
				case ' ': 
					// signal regardless? 
					// command_mtx = (command_mtx+1) % 2;
					command_mtx = 1; 
					break;
				case 'e': 
					command_mtx = 0;
					running = 0;
					break;
				case 'r': 
					command_mtx = -1; 
					break; 
				default: 
					// Invalid commands are ignored.
					break;
			}
			pthread_mutex_unlock(&run_lock);	
			pthread_cond_signal(&run_cond);
		}
	}
	return 0;
}

int init_timer_thread(int isIncrement, int seconds){
	time_args* t_args = (time_args*)malloc(sizeof(time_args));
	t_args->isIncrement = isIncrement; 
	t_args->seconds = seconds; 
    if (pthread_mutex_init(&run_lock, NULL) != 0){
		printf("[ERROR]: Init mutex failed.\n");
		exit(1);
	}
	pthread_t timer;
	int error = pthread_create(&timer, NULL, time_counter, (void*)t_args);
	if (error != 0){
		printf("Timer thread cannot be created: [%s]\n", strerror(error));
		exit(1);
	} 
	UI_loop();
	pthread_join(timer, NULL); 
	free(t_args);
	return 0;
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
    switch (key){
        case 's':
            printf("stopwatch started...\n");
		    init_timer_thread(1, 0);  
            // break;
			return 0; 
        case 't':
            printf("timer started...\n");
			init_timer_thread(0, parse_time(arg));
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

