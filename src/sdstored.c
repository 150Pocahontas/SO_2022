//Server
#include "include/sdstore.h"

typedef struct struct_task{
	char *task;
	pid_t pidT;
	int status;
}*Task;

int fd_cs, fd_read_cs, fd_sc, fd_write_sc;
int **child_pids;
int size_max = 5;
int max_inst[7];
int running[7];

//Task* priority_0, priority_1, priority_2, priority_3,priority_4,priority_5;

Task* tasks;
int cur_task;
int resp_task; //responsible for an execution
char* path;

void sign_Int_handler(){
  close(fd_cs);
  close(fd_read_cs);
  close(fd_write_sc);
  close(fd_sc);
  if(fork() == 0){
    execlp("rm","rm","pipe_task_done",NULL);
  }
  if(fork() == 0){
    execlp("rm","rm","fifo_cs",NULL);
    _exit(0);
  }
  if(fork() == 0){
    execlp("rm","rm","fifo_sc",NULL);
    _exit(0);
  }
  for(int i = 0; i<3;i++){
    wait(0);
  }
  for(int i = 0; i<size_max;i++){
    //printf("pid:    %d\n",tasks[i]->pidT );
    //printf("status: %d\n",tasks[i]->status);
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
    tasks[t] -> status = 2;
    char *string = calloc(20,sizeof(char));

		if(!fork()){
			execlp("rm","rm",string,NULL);
			_exit(0);
		}
		free(string);
	}else {
		printf("[DEBUG] shouldn´t come in here\n");
	}
	write(fd_write_sc,EXIT,sizeOfExit);
}

void printOutput(int task){
  // invalid task or concluded
  if(task < 0 || !tasks[task] || tasks[task] -> status != 2){
    write(fd_write_sc, "Invalid Task\n",14);
  }
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
  write(fd_fifo,buf,res);
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
  exec_ret = execvp(exec_args[0],exec_args);
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

void info(){
	write(fd_write_sc,"./sdstore status\n",18);
	write(fd_write_sc,"./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n",105);
}

void process(char* input, char* output, char** transformations,int numT){
	write (fd_write_sc,"processing\n",12);

	int pid[numT], status[numT];
	int filds[2];

	if(pipe(filds) == -1){
		perror("Pipe not created: ");
		return;
	}

	int fdin = dup(0);
  int fdout = dup(1);

	int fd_in = open(input,O_RDONLY);
	int fd_out = open(output,O_CREAT | O_TRUNC | O_WRONLY,0666);

	for(int i = 0; i < numT; i++){
		char* aux = malloc(sizeof(char*));
		strcpy(aux,path);
		if(aux[strlen(aux)-1]!= '/') strcat(aux,"/");
		strcat(aux,transformations[i]);

		if(fork() == 0){

			dup2(fd_in,0); // read
		  dup2(fd_out,1); //write

			close(fd_in);
			close(fd_out);

			if(i== 0 && i == numT-1){
				execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}else if(i == 0){ // produtor
				close(filds[0]);
				dup2(filds[1],1);
		    close(filds[1]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			} else if(i == numT-1)  { //consumidor
				close(filds[1]);
		    dup2(filds[0],0);
		    close(filds[0]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}else{ //consumidor e depois produtor
		    dup2(filds[0],0);
		    close(filds[0]);
				dup2(filds[1],1);
		    close(filds[1]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}
		}else{
			if (i != numT - 1) close(filds[1]);
			if(i != 0 ) close(filds[0]);
			dup2(fdout,0);
		  dup2(fdin,1);
			wait(&status[i]);
			if (WIFEXITED(status[i])) {
				printf("[PAI]: filho terminou com %d\n", WEXITSTATUS(status[i]));
			}
		}
	}
	write(fd_write_sc, "concluded\n ",10);
	write(fd_write_sc,EXIT,sizeOfExit);
}

void interpreter(char* line){
	int pid, status;
 	char* string = strtok(line," ");

	if(strcmp(line,"info") == 0){
		info();
		write(1,EXIT,sizeOfExit);
		write(fd_write_sc,EXIT,sizeOfExit);
	}else	if(strcmp(line,"status") == 0){
		executingTasks();
		write(fd_write_sc,EXIT,sizeOfExit);
	}else if(strcmp(string,"proc-file")== 0){
		if((pid = fork()) == 0){
			write(fd_write_sc,"pending\n",8);
			char* file = strtok(NULL," ");
			char* outputFolder = strtok(NULL," ");
			char** transformations = malloc(sizeof(char*));
			int t;
			char* aux = malloc(sizeof(char*));
			for(t = 0; (aux = strtok(NULL," ")); t++){
				transformations[t] = malloc(sizeof(char));
				strcpy(transformations[t],aux);
			}
			process(file,outputFolder,transformations,t);
			kill(getppid(),SIGUSR1);
			task_ended();
			_exit(pid);
		}else{
			if(tasks[cur_task]){
				tasks[cur_task]->pidT = pid;
				tasks[cur_task]->status = 1;
				tasks[cur_task]->task = calloc(strlen(string),sizeof(char));
				strcpy(tasks[cur_task++]->task,string);
			}else{
				realloc_task();
				tasks[cur_task]->pidT = pid;
				tasks[cur_task]->status = 1;
				tasks[cur_task]->task = calloc(strlen(string),sizeof(char));
				strcpy(tasks[cur_task++]->task,string);
			}
			wait(&status);
			if (WIFEXITED(status)) {
				printf("[PAI]: filho terminou com %d\n", WEXITSTATUS(status));
			}
		}
	}else{
		write(fd_write_sc,"Unkown operator\n",16);
		write(fd_write_sc,EXIT,sizeOfExit);
	}
}

int read_conf(int fd_config){
	char buf[15];
	int c;

	for(c=0;readln(fd_config,buf,15) > 0;c++){
		char* string = strtok(buf," ");

		if(strcmp(string,"nop") == 0){
			max_inst[0] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"bcompress")== 0){
			max_inst[1] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"bdecompress")== 0){
			max_inst[2] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"gcompress")== 0){
			max_inst[3] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"gdecompress")== 0){
			max_inst[4] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"encrypt")== 0){
			max_inst[5] = atoi(strtok(NULL," "));
		}else if(strcmp(string,"decrypt")== 0){
			max_inst[6] = atoi(strtok(NULL," "));
		}else{
			write(1,"Ficheiro de configuração incorreto\n",37);
			return -1;
		}
	}

	bzero(buf,15);

	if(c!=7){
		write(1,"Ficheiro de configuração incorreto\n",37);
		return -1;
	}

	return 0;
}

int main(int argc, char** argv){
	init_task();
	cur_task = 0;

	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,sign_Int_handler);

	char* buf = malloc(MAX_LINE_SIZE*sizeof(char*));
	int bread;
	int fd_config;
	path = argv[2];

	if(argc < 3){
		write(1,"[Wrong number of arguments]: The program must receive 2 arguments\n",67);
		return -1;
	}

	if((fd_config = open(argv[1],O_RDONLY)) == -1){
		perror("Primeiro argumento");
		return -1;
	}
	close(fd_config);

	if(read_conf(fd_config) ==-1) return -1;

	if((fd_read_cs = open("fifo_cs",O_RDONLY)) == -1){
		perror("open");
		return -1;
	}else	printf("[DEBUG] opened fifo Client Server for [reading]\n");

	if((fd_cs = open("fifo_cs", O_WRONLY)) == -1){
		perror("open");
		return -1;
	}else printf("[DEBUG] opened fifo Cliente Server for writing\n");

	if((fd_write_sc = open("fifo_sc",O_WRONLY)) == -1){
		perror("open");
		return -1;
	}else printf("[DEBUG] opened fifo Server Client for [writing]\n");

	if((fd_sc = open("fifo_sc", O_RDONLY)) == -1){
		perror("open");
		return -1;
	}else printf("[DEBUG] opened fifo server Client for reading\n");

	while((bread = read(fd_read_cs,buf,MAX_LINE_SIZE)) > 0){
		buf[bread] = '\0';
		write(1,buf,bread);
		write(1,"\n",strlen("\n"));
		interpreter(buf);
	  bzero(buf, MAX_LINE_SIZE * sizeof(char));
	}

	close(fd_read_cs);
	close(fd_cs);
	close(fd_sc);
	close(fd_write_sc);
	return 0;
}
