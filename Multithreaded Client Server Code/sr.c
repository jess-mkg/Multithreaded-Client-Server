#include "shared_memory.h"
#include <semaphore.h>
#include <limits.h>
#include <stdbool.h>

#define THREAD_MAX 32

struct QueryData{
    uint32_t 	qnum;
    int         slotN;
};

pthread_t           th[THREAD_MAX];
key_t               ShmKEY;
int                 ShmID;
struct Memory       *ShmPTR;
bool                queryClosed = false;

static inline uint32_t rotateRight(unsigned int i, uint32_t k) {
    unsigned int c = i;
    const uint32_t mask = (CHAR_BIT*sizeof(k) - 1);
    c &= mask;
    return (k>>c) | (k<<( (-c)&mask ));
}

void* factorise(void* ThisQueryData) {
    struct QueryData* qSlot = (struct QueryData*)ThisQueryData;
    uint32_t temp = qSlot->qnum;
    uint32_t pfact = 2;
    while(temp > 1 && !queryClosed) {
        if (temp % pfact == 0) {
            sem_wait(&mutex);
            while ((ShmPTR->serverflag[qSlot->slotN] != 0) && (!queryClosed)) {
                usleep(1000); //wait till avalible
            }
            //sem_wait(&mutex);
            ShmPTR->slot[qSlot->slotN] = pfact;
            ShmPTR->serverflag[qSlot->slotN] = 1;
            usleep(1000);
            sem_post(&mutex);
            printf("%d ", pfact);
            temp/=pfact;
        }
        else {
            pfact++;
        }
    }
    printf("\n");
    free(qSlot);
}

void  main(void)
{
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
     printf("Server has attached the shared memory ...\n");
     /////////////////////////////////////////////////////
     ShmPTR->clientflag = 0;
     for (int d = 0; d < 10; d++) {
         ShmPTR->serverflag[d] = 0;
     }
     printf("ready for input ...\n");
     while (1) {
         if (ShmPTR->clientflag == 1) {
             if (ShmPTR->command == 2) {
                 queryClosed = true;
                 printf("   Client has detacted from memory...\n");
                 ShmPTR->clientflag == 1;
                 shmdt((void *) ShmPTR); //replace with break
                 sem_destroy(&mutex);
                 printf("   Server has detached its shared memory...\n");
                 printf("   Server exits...\n");
                 exit(0);
             }
             else if (ShmPTR->number != 0){
                 uint32_t new = ShmPTR->number; //reading from number
                 printf("Input: %d\n", new);
                 int i;
                 sem_init(&mutex, 0, 1);
                 ShmPTR->clientflag = 0;
                 for (i = 0;i < 32; i++) {
                     uint32_t *a = malloc(sizeof(uint32_t));
                     uint32_t rotated = rotateRight(i, new);
                     *a = rotated;
                     int j;
                     for (j = 0; j < 10; j++){
                     	if (ShmPTR->serverflag[j] == 0) {
                            struct QueryData* qSlot = (struct QueryData*)malloc(sizeof(struct QueryData));
                            qSlot->qnum = *a;
                            qSlot->slotN = j;
                            if (pthread_create(&th[i], NULL, &factorise, (void*)qSlot) != 0) {
                                perror("Failed to create thread...");
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
                 sem_destroy(&mutex);
             }
         }
     }
     printf("   Client has detacted from memory...\n");
     shmdt((void *) ShmPTR);
     printf("   Server has detached its shared memory...\n");
     printf("   Server exits...\n");
     exit(0);
}
