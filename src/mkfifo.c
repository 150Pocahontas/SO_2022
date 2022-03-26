#include <stdio.h>
#include <sys/stat.h>

int main(){

	if (mkfifo("fifo_cs",0666) == -1){
		perror("mkfifo");
	}
	if (mkfifo("fifo_sc",0666) == -1){
		perror("mkfifo");
	}
	// creates fifo with pipe_task
	if (mkfifo("pipe_task_done",0666) == -1){
		perror("mkfifo from child");
	}
	return 0;
}
