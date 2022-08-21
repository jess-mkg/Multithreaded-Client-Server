#include<pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include <stdint.h>

int checkprime(int num)
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
            printf("2 "); //send this bacl to the client by putting into the shared memory and change flag value
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

static inline uint32_t rotateRight(unsigned int k) {
    unsigned int c = 1;
    const unsigned int mask = (CHAR_BIT*sizeof(k) - 1);
    c &= mask;
    return (k>>c) | (k<<( (-c)&mask ));
}

int trailDiv (int num){
    int i = 2;
    int k = ceil(sqrt(num));

    while (i<=k) {
        if(num % i == 0)
            return 0;
        i += 1;
    }
    return 1;
}

void* myrotate(void*arg) {
    int *iptr = (int *)arg;
    *iptr = rotateRight(*iptr);
    printf("%d\n", *iptr);
    *iptr = primefactors(*iptr);
    return NULL;
}

void* myTurn(void*arg) {
    int *iptr = (int *)arg;
    for (int i = 0; i < 3; i++) {
        sleep(1);
        printf("my turn! %d %d\n", i, *iptr);
        (*iptr)++;
    }
    return NULL;
}


void yourTurn() {
    for (int i = 0; i < 1; i++) {
        sleep(1);
        printf("your turn\n");
    }
}

int main(int argc, char const *argv[]) {
    pthread_t newthread;
    int v;
    printf("Enter a value: ");
    scanf("%d", &v);


    pthread_create(&newthread, NULL, myrotate, &v);
    //yourTurn();

    pthread_join(newthread, NULL);
    printf("Threads done: v = %d\n",v);
    return 0;
}