#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "shared_memory.h"

key_t               ShmKEY;
int                 ShmID;
struct Memory       *ShmPTR;
int p = 0;
/////////////////////////////////////////////////////
void* nextInp() {               //prints the output from the server for both the Query and the Test

    int i;
    for (i = 0; i < 10; i++) {

        if (ShmPTR->serverflag[i] == 1) {       //checks if the serverflag has been set to 1 to print output from server
            printf("Query %d: %d \n", i, ShmPTR->slot[i]);
            float newp = ShmPTR->progress[i];
            if ((newp > p) && (newp != p)) {
                printf("Query %d Progress: %f%%\n", i, newp);
                p = newp;
            }
            if (ShmPTR->finished[i] == 1) {
                printf("Query Finished!\n");
                ShmPTR->finished[i] = 0;        //sets finish back to zero
            }
            ShmPTR->serverflag[i] = 0;
            usleep(10000);
        }
    }
}
/////////////////////////////////////////////////////
int main() {
    char           command[4];      //input from user in controlled by this char
    pthread_t      response;        //server responses handled through a thread

/////////////////////////////////////////////////////

    if ((ShmKEY = ftok(".", 'x')) == -1) {      //get shared memory token value
        perror("ftok");
        exit(1);
    }
    //create the segment using the key
    if ((ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666)) == -1) {
        perror("shmget");
        exit(1);
    }
    printf("Shared memory segment created...\n");

    // attach to the segment to get a pointer to it
    ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
         perror("shmat");
         exit(1);
    }
    ShmPTR->clientflag = 0; //incase of error set all to zero
    for (int d = 0; d < 10; d++) {
        ShmPTR->serverflag[d] = 0;
    }
    printf("client has attached the shared memory...\n");

/////////////////////////////////////////////////////
    while(1) {
        usleep(2000);

        if (ShmPTR->clientflag == 0) {      //can only print if client flag is zero
            printf("Input value or 'q': ");
            scanf("%s", command);

            if (strcmp(command, "q") == 0) {

                ShmPTR->clientflag = 1;     //giving input to server
                ShmPTR->command = 2;        //exit command for server

                shmdt((void *) ShmPTR);     //close shared memory pointer connection
                printf("client has detached its shared memory...\n");
                shmctl(ShmID, IPC_RMID, NULL);
                printf("client has removed its shared memory...\n");
                printf("client exits...\n");
                exit(0);
            }
            else if (isdigit(command[0]) != 0) {
                int c = 0;
                for (int d = 0; d < 10; d++) {
                    if (ShmPTR->serverflag[d] != 0) {
                        c++;
                        if(c == 9) {
                            printf("Server busy please wait!\n");
                            break;
                       }
                   }
                }
                uint32_t num = atoi(command);
                ShmPTR->clientflag = 1;
                ShmPTR->number = num;
                printf("client has filled %d into shared memory...\n", ShmPTR->number);
            }
            else if (isdigit(command[0]) == 0) {
                int c = 0;
                for (int d = 0; d < 10; d++) {
                    if (ShmPTR->serverflag[d] != 0) {
                        c++;
                        if(c == 9) {
                            printf("Server busy please wait!\n");
                            break;
                       }
                   }
                }
                uint32_t num = atoi(command);       // change to a 32 bit number from char input command
                ShmPTR->clientflag = 1;
                ShmPTR->number = num;
                printf("client has filled %d into shared memory...\n", ShmPTR->number); //lets user know the client has shared the memory with server
            }
            else {
                printf("Invalid Input ... \n");     //input can only be q or a number
            }
        }
        if (pthread_create(&response, NULL, &nextInp, NULL) != 0) {     //responser from server handler
            perror("Failed to create thread ...");
        }

        if (pthread_detach(response) != 0) {                            //detach thread
            perror("Failed to join thread...");
        }
    }
/////////////////////////////////////////////////////
    return 0;
}
