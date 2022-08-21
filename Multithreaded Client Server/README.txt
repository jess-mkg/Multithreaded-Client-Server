Multithreaded Client Server

The aim of this project was to create a multithreaded client server system to achieve multiprocessing. The client would prompt 
the user to enter a value which would be shared with the server through shared memory. The value is then roasted 32 times, 
each rotation initiating a thread to factorise that roasted value using trial of division. The threads would find prime
factors and share each result with an allocated slot for the client to read.

In addition, the system be able to:
    ● Handle 10 simultaneous queries without blocking
    ● Follow a handshake protocol for sharing data between server and client
    ● Display the progress of each query
    ● Enable a test mode activated by entering 0 to display the functionality of the threading within the system.

This required utilising what has been taught about multithreading, IPC and
synchronization in order to achieve the wanted system.



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
