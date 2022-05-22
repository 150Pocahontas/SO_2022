#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "readln.h"

#define EXIT "//exit"
#define sizeOfExit 6
#define BUFF_SIZE 4096
#define MAX_LINE_SIZE 256
