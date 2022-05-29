//Server
#include "include/sdstore.h"

typedef struct struct_task{
	char* task;
	pid_t pidT;
	int status;
}*Task;

int fd_read_server;
int **child_pids;
int size_max = 5;
int max_inst[7];
int running[7];
int len = 0;

Task* tasks;
Task* priority_0;
Task* priority_1;
Task* priority_2;
Task* priority_3;
Task* priority_4;
Task* priority_5;

int cur_task;
int resp_task; //responsible for an execution
char* path;


void sign_Int_handler(){
  close(fd_read_server);
  if(fork() == 0){
    execlp("rm","rm","fifo_server",NULL);
    _exit(0);
  }

  for(int i = 0; i<3;i++){
    wait(0);
  }

  for(int i = 0; i<size_max; i++){
    if(tasks[i]->task) free(tasks[i]->task);
    free(tasks[i]);
    free(child_pids[i]);
  }

  free(tasks);
  free(child_pids);
  _exit(0);
}

void sigusr1SignalHandler(int signum){
  char buffer[MAX_LINE_SIZE];

  int t = atoi(buffer);

  if(tasks[t]){
    tasks[t] -> status = 2;
	}else {
		printf("[DEBUG] shouldn´t come in here\n");
	}
}

void init_task(){
	child_pids = calloc(size_max,sizeof(int*));
  tasks = calloc(size_max,sizeof(tasks));
  for(int i = 0;i<size_max;i++)
    tasks[i] = calloc(1,sizeof(struct struct_task));
}

void realloc_task(){
  tasks = realloc(tasks,2*size_max*sizeof(Task));
  child_pids = realloc(tasks,2*size_max*sizeof(int*));
  for(int i = size_max;i<2*size_max;i++)
    tasks[i]  = calloc(1,sizeof(struct struct_task));
  size_max = 2*size_max;
}

void transfState(char transf[11], int i,int fd_write_sc){
	char msg[35] = "transf ";
	strcat(msg,transf);
	strcat(msg, " ");
	char* str = malloc(sizeof(char));
	sprintf(str,"%d", running[i]);
	strcat(msg,str);
	strcat(msg,"/");
	sprintf(str,"%d", max_inst[i]);
	strcat(msg,str);
	free(str);
	strcat(msg," (running/max)");
	strcat(msg,"\n");
	write(fd_write_sc,msg,strlen(msg));
}

void executingTasks(int fd_write_sc){
	char aux[MAX_LINE_SIZE];
	int i;
	if(cur_task == 0){
		len = strlen("No tasks running\n");
		write(fd_write_sc,"No tasks running\n",len);
	}
	else{
		for(i = 0; i < cur_task; i++){
			if(tasks[i]->status == 1){
				sprintf(aux,"#%d : %s\n",i+1,tasks[i]->task);
				write(fd_write_sc,aux,strlen(aux));
			}
		}
	}

	transfState("nop",0,fd_write_sc);
	transfState("bcompress",1,fd_write_sc);
	transfState("bdecompress",2,fd_write_sc);
	transfState("gcompress",3,fd_write_sc);
	transfState("gdecompress",4,fd_write_sc);
	transfState("encrypt",5,fd_write_sc);
	transfState("decrypt",6,fd_write_sc);

	write(fd_write_sc,EXIT,strlen(EXIT));
	close(fd_write_sc);
}

int read_bytes(char* file){

	char* buffer = malloc(sizeof(char*));

	int fd = open(file,O_RDONLY);

	ssize_t bytes_read = 0;
	int c = 0;

	while((bytes_read = read(fd,buffer,1))>0)
    c += bytes_read;

  free(buffer);

	return c;
}

void process(char* input, char* output, char** transformations,int numT,int fd_write_sc){
	len = strlen("processing\n");
	write(fd_write_sc,"processing\n",len);

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
		if((pid[i] = fork()) == 0){
			dup2(fd_in,0); // read
			dup2(fd_out,1); //write
			close(fd_in);
			close(fd_out);
			if(i== 0 && i == numT-1){ //Apenas uma transformação
				execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}else if(i == 0){ // produtor -> primeira transformação
				close(filds[0]);
				dup2(filds[1],1);
		    close(filds[1]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}else if(i == numT-1){ //consumidor -> última transformação
				close(filds[1]);
		    dup2(filds[0],0);
		    close(filds[0]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}else{ //consumidor e depois produtor -> transformações do meio,
		    dup2(filds[0],0);
		    close(filds[0]);
				dup2(filds[1],1);
		    close(filds[1]);
		    execl(aux,transformations[i],NULL);
		    _exit(pid[i]);
			}
		}else{
			wait(&status[i]);
			if (i != numT - 1) close(filds[1]);
			if(i != 0 ) close(filds[0]);
			dup2(fdout,0);
		  dup2(fdin,1);
			if (WIFEXITED(status[i])) {
				printf("\nfilho terminou com %d\n", WEXITSTATUS(status[i]));
			}
		}
		free(aux);
	}

	char msg[60] = "concluded (bytes-input: ";
	int c = read_bytes(input);
	char size_str[10];
	sprintf(size_str,"%d", c);
	strcat(msg,size_str);
	strcat(msg,", bytes-output: ");
	c = read_bytes(output);
	sprintf(size_str,"%d", c);
	strcat(msg,size_str);
	strcat(msg,")\n");

	write(fd_write_sc,msg,strlen(msg));
	write(fd_write_sc,EXIT,strlen(EXIT));
	close(fd_write_sc);
}

void interpreter(char* line,int fd_write_sc){
	int pid, status;

	char string[strlen(line)];
	strcpy(string,line);
 	strtok(line," ");

	if(strcmp(line,"info") == 0){
		if((pid = fork())==0){
			len = strlen("./sdstore status\n");
			write(fd_write_sc,"./sdstore status\n",len);
			len = strlen("./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2...\n");
			write(fd_write_sc,"./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2...\n",len);
			write(fd_write_sc,EXIT,strlen(EXIT));
			close(fd_write_sc);
		}else{
			pid_t pid_filho = wait(&status);
			if (WIFEXITED(status)) {
				printf("[PAI]: filho %d terminou com %d\n",pid_filho, WEXITSTATUS(status));
			}
		}
	}else	if(strcmp(line,"status") == 0){
		if((pid = fork())==0){
			executingTasks(fd_write_sc);
		}else{
			pid_t pid_filho = wait(&status);
			if (WIFEXITED(status)) {
				printf("[PAI]: filho %d terminou com %d\n",pid_filho, WEXITSTATUS(status));
			}
		}
	}else if(strcmp(line,"proc-file")== 0){
		if((pid = fork()) == 0){
			len = strlen("pending\n");
			write(fd_write_sc,"pending\n",len);
			char* file = strtok(NULL," ");
			char* outputFolder = strtok(NULL," ");
			char** transformations = malloc(sizeof(char*));
			int t;
			char* aux = malloc(sizeof(char*));
			for(t = 0; (aux = strtok(NULL," ")); t++){
				transformations[t] = malloc(sizeof(char*));
				strcpy(transformations[t],aux);
			}
			sleep(10);
			process(file,outputFolder,transformations,t,fd_write_sc);
			kill(getppid(),SIGUSR1);
			_exit(pid);
		}else{
			if(tasks[cur_task]){
				tasks[cur_task]->pidT = pid;
				tasks[cur_task]->status = 1; // runing
				tasks[cur_task]->task = calloc(strlen(string),sizeof(char));
				strcpy(tasks[cur_task++]->task,string); //adiciona a nova tarefa e concatena o cur_task
			}else{
				realloc_task();
				tasks[cur_task]->pidT = pid;
				tasks[cur_task]->status = 1;
				tasks[cur_task]->task = calloc(strlen(string),sizeof(char));
				strcpy(tasks[cur_task++]->task,string);
			}
			pid_t pid_filho = wait(&status);
			if (WIFEXITED(status)) {
				printf("[PAI]: filho %d terminou com %d\n",pid_filho, WEXITSTATUS(status));
			}
		}
	}else{
		len = strlen("Unkown operator\n");
		write(1,"Unkown operator\n",len);
		write(fd_write_sc,"Unkown operator\n",len);
		write(fd_write_sc,EXIT,strlen(EXIT));
		close(fd_write_sc);
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
			write(1,"[1]Ficheiro de configuracao incorreto\n",38);
			return -1;
		}
	}

	bzero(buf,15);

	if(c!=7){
		write(1,"[2]Ficheiro de configuracao incorreto\n",38);
		return -1;
	}
	return 0;
}

void open_fifo_client(char* pid_client){
	int fd_read_cs, fd_write_sc;

	char* buf = malloc(MAX_LINE_SIZE*sizeof(char*));

	if((fd_read_cs = open(pid_client,O_RDONLY)) == -1){
		perror("open");
		return;
	}else printf("[DEBUG] opened fifo %s for [reading]\n",pid_client);

	//int bread = 0;
	read(fd_read_cs,buf,MAX_LINE_SIZE);
	printf("%s\n",buf);
	close(fd_read_cs);

	if((fd_write_sc = open(pid_client,O_WRONLY)) == -1){
		perror("open");
		return;
	}else printf("[DEBUG] opened fifo %s for [writing]\n",pid_client);

	interpreter(buf,fd_write_sc);
	free(buf);
}

int main(int argc, char** argv){

	signal(SIGUSR1,sigusr1SignalHandler);
	signal(SIGINT,sign_Int_handler);

	char* buf = malloc(MAX_LINE_SIZE*sizeof(char*));
	int fd_config;
	path = argv[2];

	if(argc < 3){
		write(1,"[Wrong number of arguments]: The program must receive 2 arguments\n",66);
		return -1;
	}

	if((fd_config = open(argv[1],O_RDONLY)) == -1){
		perror("Primeiro argumento");
		return -1;
	}

	if(read_conf(fd_config) == -1) return -1;
	close(fd_config);

	init_task();
	cur_task = 0;

	if (mkfifo("fifo_server",0666) == -1){
		perror("mkfifo");
	}

	//Abre o fifo para leitura
	if((fd_read_server = open("fifo_server",O_RDONLY)) == -1){
		perror("open");
		return -1;
	}else	printf("[DEBUG] opened fifo Server for [reading]\n");

	while(1){
		if(read(fd_read_server,buf,MAX_LINE_SIZE) > 0)
			open_fifo_client(buf);
	  bzero(buf, MAX_LINE_SIZE * sizeof(char));
	}

	/*while(1){
		if(read(fd_read_server,buf,MAX_LINE_SIZE) > 0)
		{
			if(fork() == 0){
				if((fd_read_cs = open(buf,O_RDONLY)) == -1){
		      perror("open");
		      return -1;
		    }else printf("[DEBUG] opened fifo %s for [reading]\n",buf);

				if((fd_write_sc = open(buf,O_WRONLY)) == -1){
					perror("open");
					return -1;
				}else printf("[DEBUG] opened fifo %s for [writing]\n",buf);

				bzero(buf, MAX_LINE_SIZE * sizeof(char));

				read(fd_read_cs,buf,MAX_LINE_SIZE);
				close(fd_read_cs);
				interpreter(buf);

				close(fd_write_sc);
				_exit(0);
			}
		  bzero(buf, MAX_LINE_SIZE * sizeof(char));
		}
	}*/
	execlp("rm","rm","fifo_server",NULL);
	close(fd_read_server);
	return 0;
}
