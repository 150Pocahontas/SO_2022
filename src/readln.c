#include "include/readln.h"

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
