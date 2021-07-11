CC=gcc 
CFLAGS = -largp
# DEPS = 

clitime: clitime.c
	$(CC) -o clitime $(CFLAGS) clitime.c 
	
