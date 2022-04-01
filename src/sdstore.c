//client
#include "include/sdstored.h"

int main(int argc, char** argv){
  //int res;
  //char buf[MAX_LINE_SIZE];
  int pid, res, fd_write_cs, fd_read_sc;
  char buf[MAX_LINE_SIZE];

  // open named pipe for write (Cliente - Server)
  if((fd_write_cs = open("fifo_cs",O_WRONLY)) == -1){
    perror("open"); return -1;
  }else{
    printf("[DEBUG] fifo client Server for writing opened\n");
  }

   // open named pipe for read (Server - Cliente)
  if((fd_read_sc = open("fifo_sc",O_RDONLY)) == -1){
    perror("open"); return -1;
  }else{
    printf("[DEBUG] fifo client Server for reading opened\n");
  }

  bzero(buf, MAX_LINE_SIZE * sizeof(char));
  //child processo
  if((pid = fork()) == 0){
    if(argc == 1){
      write(fd_write_cs,"info",4);
    }else if(argc == 2){
      write(fd_write_cs,argv[1],strlen(argv[1]));
    }else if(argc > 2){
      for (int i = 1; i<argc;i++){
        write(fd_write_cs,argv[i],strlen(argv[i]));
        write(fd_write_cs," ",strlen(" "));
      }
    }
    _exit(0);
  }else{ //father process -> writes to bash what
    while((res = read(fd_read_sc,buf,MAX_LINE_SIZE)) > 0){ // escrever tudo que vem do pipe sv->cl no terminal
      if(strcmp(buf+res-sizeOfExit,EXIT) == 0) {
        write(1,buf,res-sizeOfExit);
        bzero(buf, MAX_LINE_SIZE * sizeof(char));
      }else{
        write(1,buf,res);
        bzero(buf, MAX_LINE_SIZE * sizeof(char));
      }
    }
  }
  close(fd_write_cs);
  close(fd_read_sc);
  return 0;
}
