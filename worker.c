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
#include "definitions.h"

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

    int exitFlag = 0;
    do{
        sem_wait(sem);
            // if (msgShmPtr[0] == getpid()){
                printf("Child(%d) has determined it was scheduled.\n", getpid());
                msgShmPtr[0] = -1;
                msgShmPtr[1] = -1;
                exitFlag = 1;
            // }
        sem_post(sem);
    }while(exitFlag == 0);
    printf("Child %d exiting\n", getpid());
    closeProgram();
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    shmdt(msgShmPtr);
    shmdt(PCBShmPtr);
    shmdt(clockShmPtr);
    exit(0);
}