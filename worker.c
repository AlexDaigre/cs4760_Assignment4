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

    int myPCBIndex;
    int i;
    for (i=0; i<maxProcesses; i++){
        if (PCBShmPtr[i].pid == getpid()){
            myPCBIndex = i;
        }
    }
    printf("found my pcb\n");

    int exitFlag = 0;
    do{
        sem_wait(sem);
            if (msgShmPtr[0] == getpid()){
                printf("scheduled");
                int timeToRun;
                int timeToEndFrac;
                int timeToEndSec = 0;
                if ((rand() % 2) >= 1){
                    timeToRun = (rand() % msgShmPtr[2]);
                    timeToEndFrac = timeToRun + clockShmPtr[1];
                } else {
                    timeToEndFrac = msgShmPtr[2] + clockShmPtr[0];
                }
                while (timeToEndFrac >= 1000000000){
                    timeToEndFrac -= 1000000000;
                    timeToEndSec++;
                }
                printf("Child(%d) begun running at %d:%d it will run untill %d:%d.\n", getpid(), clockShmPtr[0], clockShmPtr[1], timeToEndSec, timeToEndFrac);

                while (!((timeToEndSec <= msgShmPtr[0]) || ((timeToEndSec == msgShmPtr[0]) && (timeToEndFrac <= msgShmPtr[1])))) {}
                
                PCBShmPtr[myPCBIndex].timeUsedDurringLastBurst = timeToEndFrac;
                PCBShmPtr[myPCBIndex].totalCpuTimeUsed += timeToEndFrac;
               
                msgShmPtr[2] = 1;
                
                printf("Child(%d) has finished running and it will ", getpid());

                if(PCBShmPtr[myPCBIndex].totalCpuTimeUsed > 500){    
                    if (rand() % 2 >= 1){
                        printf("close.\n");
                        closeProgram();
                    }
                }
                printf("requeue.\n");
            }
        sem_post(sem);
    }while(exitFlag == 0);
    // printf("Child %d exiting\n", getpid());
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