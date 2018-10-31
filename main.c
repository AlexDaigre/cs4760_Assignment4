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
#include "sharedMemory.h"
#include "definitions.h"
#include "queue.h"

void childClosed(int sig);
void closeProgramSignal(int sig);
void closeProgram();
static void interrupt(int sig, siginfo_t* info, void* context);
int setTimer(double sec);
int setInterrupt();
void setupOutputFile();
void createProcesses();
void advanceTime();

int* clockShmPtr;
int* msgShmPtr;
struct ProcessControlBlock* PCBShmPtr;
sem_t* sem;

int createNextProcessAt = -1;

int currentProcesses = 0;

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
    clockShmPtr = setupSharedClock();
    clockShmPtr[0] = 0;
    clockShmPtr[1] = 0;

    msgShmPtr = setupMsgCenter();
    msgShmPtr[0] = -1;
    msgShmPtr[1] = -1;

    PCBShmPtr = setupSharedPCBs();
    sem = setupSemaphore();

    //start the end program timer
    // if (setInterrupt() == -1){
    //     printf("Failed to set up SIGPROF handler.\n");
    //     closeProgram();
    // }

    // if (setTimer(maxRunTime) == -1){
    //     printf("Failed to set up SIGPROF timer.\n");
    //     closeProgram();
    // }

    while(1==1){
        // printf("looping!\n");
        if ((currentProcesses < maxProcesses)){
            // printf("Creating process!\n");
            createProcesses();
        }
        if ((msgShmPtr[0] < 0) && (msgShmPtr[1] < 0)){
            pid_t nextProcess = nextProcessToschedule();
            if (nextProcess > 0){ 
                // printf("High priority 1: %d\n", highPriorityQueue[1]);
                msgShmPtr[0] = nextProcess;
                msgShmPtr[1] = timeQuantum; 
                printf("Scheduled %d for %d\n", msgShmPtr[0], msgShmPtr[1]);
            }
        }
        advanceTime();
    }

    closeProgram();
}

void childClosed(int sig){
    pid_t closedChild = wait(NULL);      
    int closedPCB = -1;
    int i;
    for(i=0; i<maxProcesses; i++){
        if (PCBShmPtr[i].pid == closedChild){
            closedPCB = i;
            printf("closedPCB %d\n", closedPCB);
            break;
        }
    }
    avaliblePCBs[i] = 0;
    removeFromQueue(closedChild);
    if (msgShmPtr[0] == closedChild){
        msgShmPtr[0] = -1;
        msgShmPtr[1] = -1; 
    }
    currentProcesses--;
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
            int priority = (rand() % 100) > 90 ? 0 : 1;
            newPCB.priority = priority;
            newPCB.timeUsedDurringLastBurst[0] = 0;
            newPCB.timeUsedDurringLastBurst[1] = 0;
            newPCB.totalCpuTimeUsed[0] = 0;
            newPCB.totalCpuTimeUsed[1] = 0;
            newPCB.totalTimeInSystem[0] = 0;
            newPCB.totalTimeInSystem[1] = 0;
            avaliblePCBs[i] = 1;
            PCBShmPtr[i] = newPCB;
            createNextProcessAt = -1;
            addToQueue(newForkPid, priority);
            currentProcesses++;
        }
    }
}

void advanceTime(){
    clockShmPtr[0] += 1;
    clockShmPtr[1] += rand() % 1000;
    // clockShmPtr[1] += 100;
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

void closeProgramSignal(int sig){
    closeProgram();
}

int setInterrupt(){
    struct sigaction act;
    act.sa_sigaction = &interrupt;
    act.sa_flags = 0;
    return ((sigemptyset(&act.sa_mask) == -1) || (sigaction(SIGALRM, &act, NULL) == -1));
}

static void interrupt(int signo, siginfo_t* info, void* context){
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
    shmctl(msgShmId, IPC_RMID, NULL);
    // shmdt(msgShmPtr);
    shmctl(PCBShmId, IPC_RMID, NULL);
    // shmdt(PCBShmPtr);
    shmctl(clockShmId, IPC_RMID, NULL);
    // shmdt(clockShmPtr);
    sem_unlink(SHMNAME);
    fclose(outputFile);
    // printf("Exiting gracefully.\n");
    exit(0);
}