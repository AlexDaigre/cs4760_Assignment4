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
#include "ProcessControlBlock.h"

void childClosed(int sig);
void closeProgramSignal(int sig);
void closeProgram();
void setupSharedClock();
void setupMsgCenter();
void setupSharedPCBs();
void setupSemaphore();
void setupOutputFile();

int clockShmId;
int* clockShmPtr;

int msgShmId;
int* msgShmPtr;

int PCBShmId;
int* PCBShmPtr;

sem_t* sem;

int totalProcesses = 0;
int avaliblePCBs[20] = {0};
FILE* outputFile;

int main (int argc, char *argv[]) {
    //set signals
    signal(SIGCHLD, childClosed);
    signal(SIGINT, closeProgramSignal);

    //set default values and get command line inputs
    int c;
    int maxRunTime = 20;
    char* logFile = "logFile.txt";

    while ((c = getopt (argc, argv, "hs:l:t:")) != -1){
        switch (c){
            case 'h':
                printf("Options:\n-h: Help\n-l: The given argument(string) specifies the neame of the logfile.\n-t: The given number(int) specifies the max amount of time the program will run for.\n");
                exit(0);
                break;
            case 'l':
                logFile = optarg;
                break;
            case 't':
                maxRunTime = atoi(optarg);
                break;
            default:
                printf("there was an error with arguments");
                exit(1);
                break;
        }
    }

    //setup output file
    setupOutputFile();

    //display run parameters
    printf("Log file name: %s\n", logFile);
    fprintf(outputFile, "Log file name: %s\n", logFile);
    printf("Max run time: %d\n", maxRunTime);
    fprintf(outputFile, "Max run time: %d\n", maxRunTime);

    //Intilize various shared memory
    setupSharedClock();
    setupMsgCenter();
    setupSharedPCBs();
    setupSemaphore();


    closeProgram();
}

void setupOutputFile(){
    char* logFile = "logFile.txt";
    outputFile = fopen(logFile, "w");
    if (outputFile == NULL){
        printf("Failed to open output file.\n");
        closeProgram();
    }
}

void setupSharedClock(){
    key_t sharedClockKey;
    if (-1 != open("/tmp/daigreTmp677543", O_CREAT, 0777)) {
        sharedClockKey = ftok("/tmp/daigreTmp677543", 0);
     } else {
        printf("ftok error in parrent\n");
        exit(1);
    }

    clockShmId = shmget(sharedClockKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (clockShmId < 0) {
        printf("shmget error in parrent\n");
        exit(1);
    }

    clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((long) clockShmPtr == -1) {
        printf("shmat error in parrent\n");
        shmctl(clockShmId, IPC_RMID, NULL);
        exit(1);
    }

    clockShmPtr[0] = 0;
    clockShmPtr[1] = 0;
}

void setupMsgCenter(){
    key_t sharedMsgkKey;
    if (-1 != open("/tmp/daigreTmp677543", O_CREAT, 0777)) {
        sharedMsgkKey = ftok("/tmp/daigreTmp677543", 1);
     } else {
        printf("ftok error in parrent\n");
        exit(1);
    }

    msgShmId = shmget(sharedMsgkKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (msgShmId < 0) {
        printf("shmget error in parrent\n");
        exit(1);
    }

    msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((long) msgShmPtr == -1) {
        printf("shmat error in parrent\n");
        shmctl(msgShmId, IPC_RMID, NULL);
        exit(1);
    }

    msgShmPtr[0] = -1;
    msgShmPtr[1] = -1;
}

void setupSharedPCBs(){
    key_t sharedPCBKey;
    if (-1 != open("/tmp/daigreTmp677543", O_CREAT, 0777)) {
        sharedPCBKey = ftok("/tmp/daigreTmp677543", 2);
     } else {
        printf("ftok error in parrent\n");
        exit(1);
    }

    PCBShmId = shmget(sharedPCBKey, sizeof(struct ProcessControlBlock)*20, IPC_CREAT | 0666);
    if (PCBShmId < 0) {
        printf("shmget error in parrent\n");
        exit(1);
    }

    PCBShmPtr = (int *) shmat(PCBShmId, NULL, 0);
    if ((long) PCBShmPtr == -1) {
        printf("shmat error in parrent\n");
        shmctl(PCBShmId, IPC_RMID, NULL);
        exit(1);
    }

    PCBShmPtr[0] = 0;
    PCBShmPtr[1] = 0;
}

void setupSemaphore(){
    #define SNAME "/daigreSem432098786"
    sem = sem_open(SNAME, O_CREAT, 0644, 100);
    
    if (sem == SEM_FAILED) {
        perror("Failed to open semphore for empty");
        closeProgram();
    }
}

void childClosed(int sig){
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    shmctl(clockShmId, IPC_RMID, NULL);
    shmdt(clockShmPtr);
    sem_unlink(SNAME);
    fclose(outputFile);
    exit(0);
}