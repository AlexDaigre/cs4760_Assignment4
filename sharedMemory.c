#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/time.h>
#include "sharedMemory.h"
#include "ProcessControlBlock.h"
#include "definitions.h"

int* setupSharedClock(){
    key_t sharedClockKey;
    if (-1 != open(SHMNAME, O_CREAT, 0777)) {
        sharedClockKey = ftok(SHMNAME, CLOCKVAR);
     } else {
        printf("ftok error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    clockShmId = shmget(sharedClockKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (clockShmId < 0) {
        printf("shmget error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    int* clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((long) clockShmPtr == -1) {
        printf("shmat error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        shmctl(clockShmId, IPC_RMID, NULL);
        exit(1);
    }

    return clockShmPtr;
}

int* setupMsgCenter(){
    key_t sharedMsgkKey;
    if (-1 != open(SHMNAME, O_CREAT, 0777)) {
        sharedMsgkKey = ftok(SHMNAME, MSGVAR);
     } else {
        printf("ftok error in parrent: setupMsgCenter\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    msgShmId = shmget(sharedMsgkKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (msgShmId < 0) {
        printf("shmget error in parrent: setupMsgCenter\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    int* msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((long) msgShmPtr == -1) {
        printf("shmat error in parrent: setupMsgCenter\n");
        printf("Error: %d\n", errno);
        shmctl(msgShmId, IPC_RMID, NULL);
        exit(1);
    }

    return msgShmPtr;
}

struct ProcessControlBlock* setupSharedPCBs(){
    key_t sharedPCBKey;
    if (-1 != open(SHMPCBNAME, O_CREAT, 0777)) {
        sharedPCBKey = ftok(SHMPCBNAME, PCBVAR);
     } else {
        printf("ftok error in parrent: setupSharedPCBs\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    PCBShmId = shmget(sharedPCBKey, sizeof(struct ProcessControlBlock)*maxProcesses, IPC_CREAT | 0666);
    if (PCBShmId < 0) {
        printf("shmget error in parrent: setupSharedPCBs\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    //(void *) (int *)?
    struct ProcessControlBlock* PCBShmPtr = (struct ProcessControlBlock *) shmat(PCBShmId, NULL, 0);
    if ((long) PCBShmPtr == -1) {
        printf("shmat error in parrent: setupSharedPCBs\n");
        printf("Error: %d\n", errno);
        shmctl(PCBShmId, IPC_RMID, NULL);
        exit(1);
    }

    return PCBShmPtr;
}

sem_t* setupSemaphore(){
    sem_t* sem = sem_open(SEMNAME, O_CREAT, 0644, 100);
    
    if (sem == SEM_FAILED) {
        perror("Failed to open semphore for empty");
        exit(1);
    }

    return sem;
}