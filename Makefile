CC=gcc
CCFLAGS = 
OS = $(shell uname -s)
# DEPS =

ifeq ($(OS),Darwin)
	CCFLAGS += -largp
endif

#clitime: clitime.c
#	$(CC) -o clitime $(CCFLAGS) clitime.c 

test: clitime.c
	$(CC) -o clitime_test $(CCFLAGS) clitime.c 
	
clean: 
	rm clitime
