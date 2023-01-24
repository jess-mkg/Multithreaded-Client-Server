#include "shared_memory.h"
#include <semaphore.h>
#include <limits.h>

#define THREAD_MAX 32


pthread_mutex_t     mutex;
sem_t               semaphore;

static inline uint32_t rotateRight(unsigned int i, uint32_t k) {
    unsigned int c = i;
    const uint32_t mask = (CHAR_BIT*sizeof(k) - 1);
    c &= mask;
    return (k>>c) | (k<<( (-c)&mask ));
}

int checkprime(int num) //trial of division.
{
    if ((num == 2)||(num == 3))
        return (1);
    else
    {
        for (int i = 3; i <num; i = i + 2)
        {
            if (num % i == 0)
                return (0);
        }
        return 1;
    }
}

//function to find the prime factors
int primefactors(int num)
{
    int temp;
    temp = num;
    while(temp != 0 && (temp % 2 == 0))
    {
        if (temp % 2 == 0)
        {
            printf("2 "); //send this back to the client by putting into the shared memory and change flag value
            temp/=2;
        }
    }
    for (int k = 3; k <= temp && temp != 0; k = k + 2)
    {
        if (checkprime(k)==1)
        {
            while ((temp != 0) && (temp % k == 0))
            {
                printf("%d ",k); //send this bacl to the client
                temp/=k;
            }
        }
    }
    printf("\n");
}

void* factorise(void* arg) {
    sleep(1);
    sem_wait(&semaphore);
    uint32_t num = *(int*)arg;

    int temp;
    temp = num;
    while(temp != 0 && (temp % 2 == 0))
    {
        if (temp % 2 == 0)
        {
            ShmPTR->slot[0] = 2;
            ShmPTR->serverflag[0] = 1;
            printf("2 "); //send this back to the client by putting into the shared memory and change flag value
            temp/=2;
        }
    }
    for (int k = 3; k <= temp && temp != 0; k = k + 2)
    {
        if (checkprime(k)==1)
        {
            while ((temp != 0) && (temp % k == 0))
            {
                ShmPTR->slot[0] = k;
                ShmPTR->serverflag[0] = 1;
                printf("%d ",k); //send this bacl to the client
                temp/=k;
            }
        }
    }
    printf("\n");
    sem_post(&semaphore);
    free(arg);
}

void  main(void)
{
    pthread_mutex_t     mutex;
    pthread_t           th[THREAD_MAX];
    key_t               ShmKEY;
    int                 ShmID;
    struct Memory       *ShmPTR;
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
     printf("ready for input ...\n");
     pthread_mutex_init(&mutex, NULL);
     sem_init(&semaphore, 0, 1);
     while (1) {
         if (ShmPTR->clientflag == 1) {
             //sleep(1);
             if (ShmPTR->command == 2) {
                 printf("   Client has detacted from memory...\n");
                 ShmPTR->clientflag == 1;
                 shmdt((void *) ShmPTR);
                 printf("   Server has detached its shared memory...\n");
                 printf("   Server exits...\n");
                 exit(0);
             }
             else {
                 uint32_t new = ShmPTR->number; //reading from number
                 printf("%d\n", new);
                 ShmPTR->clientflag = 0;
                 int i;
                 for (i = 0;i < 32; i++) {
                     uint32_t *a = malloc(sizeof(uint32_t));
                     uint32_t rotated = rotateRight(i, new);
                     *a = rotated;
                     printf("----%d----\n", rotated);
                     if (pthread_create(&th[i], NULL, &factorise, a) != 0) {
                         perror("Failed to create thread...");
                     }
                 }
                 for (i = 0;i < 32; i++) {
                     if (pthread_join(th[i], NULL) != 0) {
                         perror("Failed to join thread...");
                     }
                 }
             }
         }
     }
     printf("   Client has detacted from memory...\n");
     shmdt((void *) ShmPTR);
     printf("   Server has detached its shared memory...\n");
     printf("   Server exits...\n");
     exit(0);
}
