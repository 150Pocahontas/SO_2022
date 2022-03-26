//Server
#include "include/sdstored.h"

typedef struct struct_task{
	char *task;
	pid_t pidT;
	int status;
}*Task;

int fd_cs, fd_read_cs, fd_sc, fd_write_sc;
int **child_pids;
int size_max = 20;

Task* tasks;
int cur_task;
int resp_task; //responsible for an execution

void signIntHandler(){
  close(fd_cs);
  close(fd_read_cs);
  close(fd_write_sc);
  close(fd_sc);
  if(fork() == 0){
    execlp("rm","rm","pipe_task_done",NULL);
  }
  if(fork() == 0){
    execlp("rm","rm","pipe_task_executionTime",NULL);
    _exit(0);
  }
  if(fork() == 0){
    execlp("rm","rm","pipe_task_inactivityTime",NULL);
    _exit(0);
  }
  if(fork() == 0){
    execlp("rm","rm","fifo-cl-sv",NULL);
    _exit(0);
  }
  if(fork() == 0){
    execlp("rm","rm","fifo-sv-cl",NULL);
    _exit(0);
  }
  for(int i = 0; i<3;i++){
    wait(0L);
  }
  for(int i = 0; i<size_max;i++){
    //printf("pid:    %d\n",tasks[i]->pidT );
    //printf("status: %d\n",tasks[i]->status );
    //printf("task: %s\n",tasks[i]->task);
    //printf("start offset: %d\n",tasks[i]->o_start );
    //printf("end offset: %d\n",tasks[i]->o_size );
    if(tasks[i]->task) free(tasks[i]->task);
    free(tasks[i]);
    free(child_pids[i]);
  }
  free(tasks);
  free(child_pids);
  _exit(0);
}

//The default action is to terminate the process
void sigusr1SignalHandler(int signum){
  int fd_fifo, res = 0;
  char buffer[MAX_LINE_SIZE];


  if((fd_fifo = open("pipe_task_done",O_RDONLY)) == -1)
    perror("open-2");

  res = read(fd_fifo,buffer,MAX_LINE_SIZE);
  buffer[res] = '\0';
  close(fd_fifo);

  int t = atoi(buffer);

  if (tasks[t]){
    tasks[t] -> status = 2; //concluded
    char *string = calloc(20,sizeof(char));

		if(!fork()){
			execlp("rm","rm",string,NULL);
			_exit(0);
		}
		free(string);
	}
	else { // realloc do array
		printf("[DEBUG] shouldn´t come in here\n");
	}
	write(fd_write_sc,EXIT,sizeOfExit);
}

void sigExecutionAlarmHandler(int signum){
  kill(getppid(),SIGUSR2);
  int fd_fifo, res;
  char buf[MAX_LINE_SIZE];

  // child waits til father does´t create fifo,
  while ((fd_fifo = open("pipe_task_executionTime",O_WRONLY)) == -1);

  res = sprintf(buf,"%d",cur_task);
  write(fd_fifo,buf,res);

  int i = 0;
  while (child_pids[cur_task][i]){
    kill(child_pids[cur_task][i++],SIGKILL);
  }

  close(fd_fifo);
  free(child_pids[cur_task]);
  _exit(0);
}

void sigQuitInactivity(int signum){
	int currentTask;
	int fd_fifo;
	int res;
	char buffer[MAX_LINE_SIZE];
	int status;

	if((fd_fifo = open("pipe_task_inactivityTime",O_RDONLY)) == -1){ // abre extremo de leitura
		perror("open-3");
	}

	res = read(fd_fifo,buffer,MAX_LINE_SIZE); // lê número da tarefa

	buffer[res] = '\0';

	currentTask = atoi(buffer); // transforma em um int

	tasks[currentTask]->status = 3;

	waitpid(tasks[currentTask]->pidT, &status, 0);

	write(fd_write_sc,EXIT,sizeOfExit);

	close(fd_fifo);

	char *string = calloc(20,sizeof(char));
	if(!fork()){
		sprintf(string,"temp_out%d.txt",currentTask+1);
		execlp("rm","rm",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
	}
	free(string);
}


void printOutput(int task){
  // invalid task or concluded
  if(task < 0 || !tasks[task] || tasks[task] -> status != 2){
    write(fd_write_sc, "Invalid Task\n",14);
  }else{
    //
  }
}

// fathers handler, to open comunication fifo with child, so he can pass the task
void execution_timeHandler(int signum){
  int currentTask, fd_fifo, res, status;
  char buffer[MAX_LINE_SIZE];

  if((fd_fifo = open("pipe_task_executionTime",O_RDONLY)) == -1){
		perror("open-3");
	}

  // gets task number
  res = read(fd_fifo,buffer,MAX_LINE_SIZE);

  buffer[res] = '\0';
	currentTask = atoi(buffer);
	tasks[currentTask]->status = 4; //max execution_timeHandler

  waitpid(tasks[currentTask]->pidT,&status,0);

  char *string = calloc(20,sizeof(char));

  if(!fork()){
    sprintf(string,"temp_out%d.txt",currentTask+1);
		execlp("rm","rm",string,NULL); // 1 - > fd_sv_cl_write
		_exit(0);
  }
  free(string);
  write(fd_write_sc,EXIT,sizeOfExit);
  close(fd_fifo);
}

void killProcessUSR1_handler(int signum){
	int i = 0;
	while(child_pids[cur_task][i]){
		kill(child_pids[cur_task][i++],SIGKILL);
	}
	free(child_pids[cur_task]);
	_exit(0);
}

void realloc_task(){
  tasks = realloc(tasks,2*size_max*sizeof(Task));
  child_pids = realloc(tasks,2*size_max*sizeof(int*));
  for(int i = size_max;i<2*size_max;i++)
    tasks[i]  = calloc(1,sizeof(struct struct_task));
  size_max = 2*size_max;
}

void init_task(){
  tasks = calloc(size_max,sizeof(tasks));
  child_pids = calloc(size_max,sizeof(int*));
  for(int i = 0;i<size_max;i++)
    tasks[i] = calloc(1,sizeof(struct struct_task));
}

void task_ended(){
  int fd_fifo;
  int res;

  char buf[MAX_LINE_SIZE];

  while((fd_fifo = open("pipe_task_done",O_WRONLY)) == -1);
  res = sprintf(buf,"%d",cur_task);
  write(fd_fifo,buf,res); // child comunicates father his task
  close(fd_fifo);
}

int end_task(int task){
  if (task <= 0 || !tasks[task-1] || tasks[task-1]->status != 1){
    write(fd_write_sc,"Invalid Task\n", 14);
    return 0;
  }

  int pid = tasks[task-1]->pidT;
  int status;
  kill(pid,SIGUSR1);
  waitpid(pid,&status,0);
  if (WIFEXITED(status))
    printf("Task finished with exit stauts %d\n",WEXITSTATUS(status));
  tasks[task-1]->status = 5; //killed

  char* string = calloc(20,sizeof(char));
  if(!fork()){
    sprintf(string,"temp_out%d.txt",task);
		execlp("rm","rm",string,NULL); // 1 - > fd_write_sc
		_exit(0);
	}
	free(string);
  return 1;
}

int exec_command(char* command){
  char* exec_args[20];
  char* string;
  int exec_ret = 0;
  int i = 0;

  string = strtok(command, " ");

  while (string != NULL) {
    exec_args[i] = string;
    string = strtok(NULL," ");
    i++;
  }

  exec_args[i] = NULL;
  exec_ret = execvp(exec_args[0], exec_args);
  return exec_ret;
}

void executingTasks(){
	char aux[MAX_LINE_SIZE];
	int i;
	for(i = 0; i < cur_task; i++){
		if(tasks[i]->status == 1){
			sprintf(aux,"#%d : %s\n",i+1,tasks[i]->task);
			write(fd_write_sc,aux,strlen(aux));
		}
	}
}

int interpreter(char* line){
 	int r = 1;
 	char* string = strtok(line," ");
 	int pid;

	if(strcmp(line,"stastus") == 0){
		executingTasks();
		write(fd_write_sc,EXIT,sizeOfExit);
	}else if(strcmp(string,"proc-file")==0){
			printf("hi");
	}
}

int main(int argc, char** argv){
	init_task();
	cur_task = 0;
	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,signIntHandler);
	signal(SIGUSR2,execution_timeHandler);
	signal(SIGQUIT,sigQuitInactivity);
	char buf[MAX_LINE_SIZE];
	int bread;

	if(argc == 3){
		// open named pipe for reading
		if((fd_read_cs = open("fifo_cs",O_RDONLY)) == -1){
			perror("open");
			return -1;
		}else
			printf("[DEBUG] opened fifo Client Server for [reading]\n");

		// open named pipe for writing to handle asynchronous clients
		if((fd_cs = open("fifo_cs", O_WRONLY)) == -1){
			perror("open");
			return -1;
		}else
			printf("[DEBUG] opened fifo Cliente Server for writing\n");

		// open named pipe for writing
		if((fd_write_sc = open("fifo_sc",O_WRONLY)) == -1){
			perror("open");
			return -1;
		}else
			printf("[DEBUG] opened fifo Server Client for [writing]\n");

		// open named pipe for writing to handle asynchronous clients
		if((fd_sc = open("fifo_sc", O_RDONLY)) == -1){
			perror("open");
			return -1;
		}else
			printf("[DEBUG] opened fifo server Client for reading\n");

		// reads from pipe, execute
		while((bread = read(fd_read_cs,buf,MAX_LINE_SIZE)) > 0){
			buf[bread] = '\0';
			interpreter(buf);
			write(1,buf,bread);
		  bzero(buf, MAX_LINE_SIZE * sizeof(char));
		}
		close(fd_read_cs);
		close(fd_cs);
		close(fd_sc);
		close(fd_write_sc);
	}else{
		char* string = "[Wrong number of arguments]: The program must receive 2 arguments\n";
		write(1,string,strlen(string));
	}

  return 0;
}
