#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "log.h"


void launch_process() {
	
	pid_t pid = fork();

	if (pid < 0) // Error
		log_write(CRITICAL_L, "Main: Fork failed!\n");
		
	else if (pid == 0) { // child
		log_write(INFO_L, "Child: This is me!\n");
		log_write(INFO_L, "Child: I must also close the log\n");
		log_close();
		exit(0);
	}
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 
 * 		MAIN DOWN HERE
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int main(int argc, char **argv){	
	launch_process();
	log_write(INFO_L, "Main: This is main! Waiting child\n");
	int status;
	wait(&status);
	log_write(STAT_L, "Program finished\n");
	log_close();
	return 0;
}
