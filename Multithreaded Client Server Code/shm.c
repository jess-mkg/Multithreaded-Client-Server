#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>

#define  NOT_READY  -1
#define  FILLED     0
#define  TAKEN      1

#define SEM_READER_FNAME "/mysemr"
#define SEM_WRITER_FNAME "/mysemw"

void *attachSharedMemory();
bool detachSharedMemory();
bool destroySharedMemory();
char *decimal_to_binary(int n);

struct Memory {
	int  status;
	int  data[10];
	char* clientflag;
	char serverflag[10];
	char* command;
};
