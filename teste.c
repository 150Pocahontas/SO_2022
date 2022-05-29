#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define BUFF_SIZE 4096

char readBuffer[7];
int pos_atual = 0;
int pos_final = 0;

ssize_t readchar(int fd, char *c){
  if (pos_final == pos_atual){
    pos_final = read(fd, readBuffer, 7);
    if (pos_final > 0)
      pos_atual = 0;
    else return 0;
  }
  *c = readBuffer[pos_atual++];
  return 1;
}

ssize_t readln(int fd, char* line, size_t size){

  int counter = 0;

  while(readchar(fd,&line[counter]) > 0 && counter < size){
    counter++;
    if(line[counter-1] == '\n')
      return counter;
  }
  return counter;
}

int a = 0;
int *i = &a;

int main(int argc, char** argv){
  int status;
  if((pid[i] = fork()) == 0){
    if(i== 0 && i == numT-1){ //Apenas uma transformação
      close(filds[0]);
      close(filds[1]);
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


  return 0;
}
