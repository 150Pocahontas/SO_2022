//client
#include "include/sdstore.h"

int main(int argc, char** argv){
  int status, res, fd_write_server, fd_write_cs, fd_read_sc;
  char buf[MAX_LINE_SIZE];

  bzero(buf, MAX_LINE_SIZE * sizeof(char));

  if(fork() == 0){

    //abre o fifo para escrita
    if((fd_write_server= open("fifo_server",O_WRONLY)) == -1){
      perror("open");
      return -1;
    }//else printf("[DEBUG] opened fifo Server for [writing]\n");

    char* pid_str = malloc(sizeof(char));
    sprintf(pid_str,"%d",getpid());

    if (mkfifo(pid_str,0666) == -1){
  		perror("mkfifo");
  	}

    write(fd_write_server,pid_str,strlen(pid_str));
    close(fd_write_server);

    //abre file para escrita
    if((fd_write_cs = open(pid_str,O_WRONLY)) == -1){
      perror("open");
      return -1;
    }//else	printf("[DEBUG] opened fifo %s for [writing]\n",buf);

    free(pid_str);

    if(argc == 1){
      write(fd_write_cs,"info",strlen("info"));
    }else if(argc == 2){
      write(fd_write_cs,argv[1],strlen(argv[1]));
    }else if(argc > 2){
      for (int i = 1; i<argc;i++){
        write(fd_write_cs,argv[i],strlen(argv[i]));
        write(fd_write_cs," ",strlen(" "));
      }
    }
    close(fd_write_cs);
    _exit(0);
  }else{

    pid_t terminated_pid = wait(&status);

    char* pid_str = malloc(sizeof(char));
    sprintf(pid_str,"%d",terminated_pid);

    if((fd_read_sc = open(pid_str,O_RDONLY)) == -1){
      perror("open");
      return -1;
    }//else printf("[DEBUG] opened fifo %s for [reading]\n",buf);

    while((res = readln(fd_read_sc,buf,MAX_LINE_SIZE)) > 0){
      if(strcmp(buf,EXIT) == 0) {
        //if(argc > 2) if(read(fd_read_sc,buf,MAX_LINE_SIZE));
        close(fd_read_sc);
        execlp("rm","rm",pid_str,NULL);
        return 1;
      }else{
        write(1,buf,res);
        bzero(buf, MAX_LINE_SIZE * sizeof(char));
      }
    }
  }
  return 0;
}
