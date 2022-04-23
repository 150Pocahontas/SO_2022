#include <sys/types.h>
#include <unistd.h> /* chamadas ao sistema: defs e decls essenciais */
#include <fcntl.h> /* O_RDONLY, O_WRONLY, O_CREAT, O_* */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char** argv){

  //execl("bin/sdstore-transformations/bcompress","bcompress",NULL);
  //execl("bin/sdstore-transformations/nop","nop",NULL);

  int fd_in = open(argv[3],O_RDONLY);
  int fd_out = open(argv[4],O_CREAT | O_TRUNC | O_WRONLY,0666);

  dup2(fd_in,0); // read
  dup2(fd_out,1); //write

  execl(argv[1],argv[2],NULL);
  perror("erro");

  //./bcompress | ./nop | ./encrypt
  return 0;
}
