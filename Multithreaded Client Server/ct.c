#include <pthread.h>
#include <semaphore.h>
#include "shared_memory.h"

char *decimal_to_binary(int n) {
  int c, d, t;
  char *p;
  t = 0;
  p = (char*)malloc(32+1);
  if (p == NULL)
    exit(EXIT_FAILURE);
  for (c = 31 ; c >= 0 ; c--) {
    d = n >> c;
    if (d & 1)
      *(p+t) = 1 + '0';
    else
      *(p+t) = 0 + '0';
    t++;
  }
  *(p+t) = '\0';
  return  p;
}


void* create_user_thread() {
    char           command[4];
    key_t          ShmKEY;
    int            ShmID;
    struct Memory  *ShmPTR;

    sem_t *sem_reader = sem_open(SEM_READER_FNAME, 0);
    if (sem_reader == SEM_FAILED) {
        perror("sem_open/reader");
        exit(1);
    }

    sem_t *sem_writer = sem_open(SEM_WRITER_FNAME, 1);
    if (sem_writer == SEM_FAILED) {
        perror("sem_open/writer");
        exit(1);
    }

    /* make the key: */
    if ((ShmKEY = ftok(".", 'x')) == -1) {
        perror("ftok");
        exit(1);
    }
    /*  create the segment: */
    if ((ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        exit(1);
    }
    printf("Shared memory segment created...\n");

    /* attach to the segment to get a pointer to it: */
    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
         perror("shmat");
         exit(1);
    }
    printf("client has attached the shared memory...\n");
    ShmPTR->status  = NOT_READY;
    while(1) {
        sem_wait(sem_reader);
        printf("Input value or 'q': ");
        scanf("%s", command);
        if (strcmp(command, "q") == 0) {
            ShmPTR->clientflag = "1";
            ShmPTR->command = "q";
            sem_post(sem_writer);
            shmdt((void *) ShmPTR);
            printf("client has detached its shared memory...\n");
            shmctl(ShmID, IPC_RMID, NULL);
            printf("client has removed its shared memory...\n");
            printf("client exits...\n");
            sem_close(sem_reader);
            sem_close(sem_writer);
            exit(0);
        }
        else if (isdigit(command[0]) != 0) {
            int num = atoi(command);
            printf("Input is an int -> %d\n", num);
            char* binary = decimal_to_binary(num);
            //num = atoi(binary);
            printf("Input as 32 bit -> %s\n", binary);
            ShmPTR->clientflag = "1";
            ShmPTR->data[0] = num;
            ShmPTR->command = "a number";
            sem_post(sem_writer);
            printf("client has filled %d shared memory...\n",
                   ShmPTR->data[0]);
            ShmPTR->status = FILLED;
            printf("Please start the server in another window...\n");
        }
        else {
            printf("Invalid Input ... \n");
        }
    }
}




int main() {

    pthread_t   user;
    int         res;

    res = pthread_create(&user, NULL, create_user_thread, NULL);
    if (res != 0) {
        printf("Error creating thread ... ");
        exit(1);
    }
    pthread_join(user, NULL);
}
