#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h> 
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "sharedMemory.h"
#include "ProcessControlBlock.h"

void closeProgramSignal(int sig);
void closeProgram();

int* clockShmPtr;
int* msgShmPtr;
struct ProcessControlBlock* PCBShmPtr;
sem_t* sem;

int main (int argc, char *argv[]) {
    int clockShmId;
    signal(SIGINT, closeProgramSignal);
    printf("Child %d started\n", getpid());

    clockShmPtr = setupSharedClock();
    msgShmPtr = setupMsgCenter();
    PCBShmPtr = setupSharedPCBs();
    sem = setupSemaphore();

    printf("CHILD clock: %d:%d\n", clockShmPtr[0], clockShmPtr[1]);

    // sem_t* sem = sem_open(SNAME, 0);
    // if (sem == SEM_FAILED) {
    //     printf("Failed to open semphore for empty");
    //     closeProgram();
    // }

    printf("Child %d exiting\n", getpid());
    closeProgram();
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    // shmdt(msgShmPtr);
    exit(0);
}