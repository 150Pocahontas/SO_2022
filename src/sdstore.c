//client
#include "include/sdstored.h"

int main(int argc, char** argv){
  int res;
  char buf[MAX_LINE_SIZE];
  int fd_write_cs, fd_read_sc;
  int pid;

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

  //bzero(buf;MAX_LINE_SIZE * sizeOf(char)); //erases data from buf

  if((pid = fork()) == 0){
    //child processo
    write(1,"hello\n",6);
    _exit(0);
  }else{
    //father process

  }
  close(fd_write_cs);
  close(fd_read_sc);
  return 0;
}
