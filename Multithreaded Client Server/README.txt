
//////////////////////////////////////////
Programs provided:

Code files          Compile method

shmServer.c         gcc -o s shmServer.c -pthread -lm
shmClient.c         gcc -o c shmClient.c -pthread
shared_memory.h

Start by compiling either program with the compile method given above.
Next you will want to call ./c first to run the client in a window, followed by
running ./s second in a separate window. This is necessary to initiate the
connection correctly.

The client window will prompt the user to input a command - either a number
or the lowercase letter 'q' to quit the window. Entering 0 will initiate the
test mode.

If 10 querys are running the user will not be able to input another NUMBER
and will be prompted to wait

segments created with IPC_CREAT | 0666, if this is of conflict to the user
please alter the values on both the client and server program to meet your needs.
