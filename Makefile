CC=gcc 
CFLAGS = -largp
# DEPS = 

timer: timer.c
	$(CC) -o timer $(CFLAGS) timer.c 
	
