#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "readln.h"

#define EXIT "//exit \n"
#define sizeOfExit 8
#define MAX_LINE_SIZE 256
