#include "shared_memory.h"
#include <semaphore.h>
#include <limits.h>
#include <stdbool.h>

#define THREAD_MAX 32
#define TEST_MAX 10
/////////////////////////////////////////////////////

struct QueryData{       //struct for each query
    uint32_t 	qnum;
    int         slotN;
    int         threadN;
};

/////////////////////////////////////////////////////
struct Memory       *ShmPTR;
pthread_t           th[THREAD_MAX];
sem_t 		        mutex;
key_t               ShmKEY;
int                 ShmID;
bool                queryClosed = false;
int                 start = 0;
/////////////////////////////////////////////////////

static inline uint32_t rotateRight(unsigned int i, uint32_t k) {
    unsigned int c = i;
    const uint32_t mask = (CHAR_BIT*sizeof(k) - 1);
    c &= mask;
    return (k>>c) | (k<<( (-c)&mask ));
}
/////////////////////////////////////////////////////
void* counter(void* arg) {

    int *iptr = (int *)arg;

    for (int t = 0; t < 10; t++){
        sem_wait(&mutex);

        while (ShmPTR->serverflag[*iptr] != 0) {
            usleep(10000); //wait till avalible
        }

        ShmPTR->slot[*iptr] = start;
        ShmPTR->serverflag[*iptr] = 1;
        start++;

        usleep(10000);
        sem_post(&mutex);
    }
    return NULL;
}
/////////////////////////////////////////////////////

void* factorise(void* ThisQueryData) {

    struct QueryData* qSlot = (struct QueryData*)ThisQueryData;
    uint32_t temp = qSlot->qnum;
    uint32_t pfact = 2;

    while(temp > 1 && !queryClosed) {
        if (temp % pfact == 0) {
            sem_wait(&mutex);

            while ((ShmPTR->serverflag[qSlot->slotN] != 0) && (!queryClosed)) {
                usleep(10000); //wait till avalible
            }

            ShmPTR->slot[qSlot->slotN] = pfact;
            ShmPTR->serverflag[qSlot->slotN] = 1;

            usleep(10000);
            sem_post(&mutex);
            temp/=pfact;
        }
        else {
            pfact++;
        }
    }
    free(qSlot);
}
/////////////////////////////////////////////////////

void  main(void) {

    // connecting to shared memory segment
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
     printf("Server has attached the shared memory ...\n");
/////////////////////////////////////////////////////
     printf("ready for input ...\n");

     //user can now give input
     while (1) {

         if (ShmPTR->clientflag == 1) { //input detected
             if (ShmPTR->command == 2) {    //user entered q thus the programm will end

                 queryClosed = true;
                 printf("   Client has detacted from memory...\n");

                 ShmPTR->clientflag == 1;

                 shmdt((void *) ShmPTR);
                 sem_destroy(&mutex);
                 printf("   Server has detached its shared memory...\n");
                 printf("   Server exits...\n");
                 exit(0);
             }

             else if (ShmPTR->number != 0){     //number input above zero
                 uint32_t new = ShmPTR->number; //reading from number
                 printf("Input: %d\n", new);    //print input to server to view behaviour
                 int i, j;
                 sem_init(&mutex, 0, 1);        //initate semaphore named mutex

                 ShmPTR->clientflag = 0;

                 for (i = 0;i < 32; i++) {
                     uint32_t *a = malloc(sizeof(uint32_t));
                     uint32_t rotated = rotateRight(i, new);
                     *a = rotated;

                     for (j = 0; j < 10; j++){  //if a serverflag slot is avalible the 32 threads will be assigned to it
                        if (ShmPTR->serverflag[j] == 0) {
                            struct QueryData* qSlot = (struct QueryData*)malloc(sizeof(struct QueryData));

                            qSlot->qnum = *a;   //
                            qSlot->slotN = j;
                            qSlot->threadN = i;
                            float numerator = qSlot->threadN;
                            ShmPTR->progress[qSlot->slotN] = (numerator/32.0)*100;

                            if (pthread_create(&th[i], NULL, &factorise, (void*)qSlot) != 0) {
                                perror("Failed to create thread...");
                            }
                            if (qSlot->threadN == 31){
                                ShmPTR->finished[qSlot->slotN] = 1;
                            }
                     		break;
                     	}
                     }
                 }

                 for (i = 0;i < 32; i++) {
                     if (pthread_join(th[i], NULL) != 0) {
                         perror("Failed to join thread...");
                     }
                 }

                 sem_destroy(&mutex);             //destroy semaphore named mutex
             }
             else if (ShmPTR->number == 0){      //test mode - zero as user input

                 printf("test\n" );
                 usleep(2000);

                 ShmPTR->clientflag = 0;

                 int thnum = 0;
                 sem_init(&mutex, 0, 1);        //initate semaphore named mutex
                 int i;

                 for (i = 0; i < 10; i++){      //create 10 threads to incriment value
                     if (pthread_create(&th[i], NULL, &counter, &thnum) != 0) {
                         perror("Failed to create thread...");
                     }
                 }
                 for (i = 0;i < 10; i++) {
                     if (pthread_join(th[i], NULL) != 0) {
                         perror("Failed to join thread...");
                     }

                }
                sem_destroy(&mutex);

                thnum = 1;
                sem_init(&mutex, 0, 1);

                for (i = 0; i < 10; i++){
                    if (pthread_create(&th[i], NULL, &counter, &thnum) != 0) {
                        perror("Failed to create thread...");
                    }
                }
                for (i = 0;i < 10; i++) {
                    if (pthread_join(th[i], NULL) != 0) {
                        perror("Failed to join thread...");
                    }
               }
               sem_destroy(&mutex);

               thnum = 2;
               sem_init(&mutex, 0, 1);

               for (i = 0; i < 10; i++){
                   if (pthread_create(&th[i], NULL, &counter, &thnum) != 0) {
                       perror("Failed to create thread...");
                   }
               }
               for (i = 0;i < 10; i++) {
                   if (pthread_join(th[i], NULL) != 0) {
                       perror("Failed to join thread...");
                   }

              }
              sem_destroy(&mutex);
             //clock_t begin = clock();
             //clock_t end = clock();
             //double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
             //printf("TIME: %f\n", time_spent);
            }
        }
     }
     printf("   Client has detacted from memory...\n");
     shmdt((void *) ShmPTR);
     printf("   Server has detached its shared memory...\n");
     printf("   Server exits...\n");
     exit(0);
}
