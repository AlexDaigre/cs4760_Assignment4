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

#define maxProcesses 18

void childClosed(int sig);
void closeProgramSignal(int sig);
void closeProgram();
void interrupt(int sig, siginfo_t* info, void* context);
int setTimer(double sec);
int setInterrupt();
void setupSharedClock();
void setupMsgCenter();
void setupSharedPCBs();
void setupSemaphore();
void setupOutputFile();
void createProcesses();
void advanceTime();

int clockShmId;
int* clockShmPtr;

int msgShmId;
int* msgShmPtr;

int PCBShmId;
struct ProcessControlBlock* PCBShmPtr;

int createNextProcessAt = -1;

sem_t* sem;

int currentProcesses = 0;

pid_t lowPriorityQueue[maxProcesses];
pid_t highPriorityQueue[maxProcesses];

int totalProcesses = 0;
int avaliblePCBs[maxProcesses] = {0};
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

    //start the end program timer
    if (setInterrupt() == -1){
        printf("Failed to set up SIGPROF handler.\n");
        closeProgram();
    }

    if (setTimer(maxRunTime) == -1){
        printf("Failed to set up SIGPROF timer.\n");
        closeProgram();
    }

    while(1==1){
        // printf("looping!\n");
        if ((currentProcesses < maxProcesses)){
            // printf("Creating process!\n");
            createProcesses();
        }
        advanceTime();
    }

    closeProgram();
}

void childClosed(int sig){
    // currentProcesses--;
    // printf("Child Closed\n");
}

void createProcesses(){
    // printf("creating child\n");
    pid_t newForkPid;

    if (createNextProcessAt < 0){
        int randNumber = (rand() % 2);
        createNextProcessAt = randNumber + clockShmPtr[0];
        printf("next process at %d seconds\n", createNextProcessAt);
    }

    if ((clockShmPtr[0] > createNextProcessAt) && (createNextProcessAt > 0)){
        printf("%d seconds reached\n", createNextProcessAt);
        int firstOpenPCB = -1;
        int i;
        for(i=0; i<maxProcesses; i++){
            if (avaliblePCBs[i] == 0){
                firstOpenPCB = i;
                printf("firstOpenPCB %d\n", firstOpenPCB);
                break;
            }
        }
        if(firstOpenPCB >= 0){
            printf("creating child\n");
            createNextProcessAt = -1;
            newForkPid = fork();
            if (newForkPid == 0){
                execlp("./worker","./worker", NULL);
                fprintf(stderr,"Failed to exec worker!\n");
                exit(1);
            }
            struct ProcessControlBlock newPCB;
            newPCB.pid = newForkPid;
            newPCB.priority = (rand() % 100) > 90 ? 0 : 1;
            newPCB.timeUsedDurringLastBurst[0] = 0;
            newPCB.timeUsedDurringLastBurst[1] = 0;
            newPCB.totalCpuTimeUsed[0] = 0;
            newPCB.totalCpuTimeUsed[1] = 0;
            newPCB.totalTimeInSystem[0] = 0;
            newPCB.totalTimeInSystem[1] = 0;
            avaliblePCBs[i] = 1;
            PCBShmPtr[i] = newPCB;
            createNextProcessAt = -1;
            currentProcesses++;
        }
    }
}

void advanceTime(){
    clockShmPtr[1]++;
    while (clockShmPtr[1] >= 1000000000){
        clockShmPtr[1] -= 1000000000;
        clockShmPtr[0]++;
        printf("%d:%d\n", clockShmPtr[0], clockShmPtr[1]);
    }
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

    clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((long) clockShmPtr == -1) {
        printf("shmat error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
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

    msgShmPtr = (int *) shmat(msgShmId, NULL, 0);
    if ((long) msgShmPtr == -1) {
        printf("shmat error in parrent: setupMsgCenter\n");
        printf("Error: %d\n", errno);
        shmctl(msgShmId, IPC_RMID, NULL);
        exit(1);
    }

    msgShmPtr[0] = -1;
    msgShmPtr[1] = -1;
}

void setupSharedPCBs(){
    key_t sharedPCBKey;
    if (-1 != open("/tmp/daigreTmp677543", O_CREAT, 0777)) {
        sharedPCBKey = ftok("/tmp/daigreTmp66755", 0);
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
    PCBShmPtr = (struct ProcessControlBlock *) shmat(PCBShmId, NULL, 0);
    if ((long) PCBShmPtr == -1) {
        printf("shmat error in parrent: setupSharedPCBs\n");
        printf("Error: %d\n", errno);
        shmctl(PCBShmId, IPC_RMID, NULL);
        exit(1);
    }
}

void setupSemaphore(){
    #define SNAME "/daigreSem432098786"
    sem = sem_open(SNAME, O_CREAT, 0644, 100);
    
    if (sem == SEM_FAILED) {
        perror("Failed to open semphore for empty");
        closeProgram();
    }
}

void closeProgramSignal(int sig){
    closeProgram();
}

int setInterrupt(){
    struct sigaction act;
    act.sa_sigaction = interrupt;
    act.sa_flags = 0;
    return ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1));
}

void interrupt(int signo, siginfo_t* info, void* context){
    closeProgram();
}

int setTimer(double sec){
    struct itimerval value;
    value.it_interval.tv_sec = sec;
    value.it_interval.tv_usec = 0;
    value.it_value = value.it_interval;
    return (setitimer(ITIMER_PROF, &value,  NULL));
}

void closeProgram(){
    shmctl(clockShmId, IPC_RMID, NULL);
    shmdt(clockShmPtr);
    sem_unlink(SNAME);
    fclose(outputFile);
    exit(0);
}