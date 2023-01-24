#include "shared_memory.h"
#include <semaphore.h>

int rotateRight(int k) {
    return (k >> 1)|(k << 32-1);
}

void  main(void)
{
     key_t          ShmKEY;
     int            ShmID;
     struct Memory  *ShmPTR;


     sem_unlink(SEM_READER_FNAME);
     sem_unlink(SEM_WRITER_FNAME);

     sem_t *sem_reader = sem_open(SEM_READER_FNAME, IPC_CREAT, 0666, 0);
     if (sem_reader == SEM_FAILED) {
         perror("sem_open/reader");
         exit(1);
     }

     sem_t *sem_writer = sem_open(SEM_WRITER_FNAME, IPC_CREAT, 0666, 1);
     if (sem_writer == SEM_FAILED) {
         perror("sem_open/writer");
         exit(1);
     }
     //////////////////////////////////////////////////
     ShmKEY = ftok(".", 'x');
     ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory ...\n");

     ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
     if ((int) ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");
     /////////////////////////////////////////////////////


     while (true) {
         sem_wait(sem_writer);
         if (ShmPTR->clientflag == "1") {
             printf("Reading: \"%s\"\n", ShmPTR->command);
             if (ShmPTR->command == "q") {
                 printf("   server has informed client data have been taken...\n");
                 ShmPTR->clientflag == "0";
                 shmdt((void *) ShmPTR);
                 printf("   server has detached its shared memory...\n");
                 printf("   server exits...\n");
                 sem_close(sem_reader);
                 sem_close(sem_writer);
                 exit(0);
             }
         }
         sem_post(sem_reader);
     }

     sem_close(sem_reader);
     sem_close(sem_writer);

     printf("   server has informed client data have been taken...\n");
     shmdt((void *) ShmPTR);
     printf("   server has detached its shared memory...\n");
     printf("   server exits...\n");
     exit(0);
}
