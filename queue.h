#ifndef QUEUE_H
#define QUEUE_H

    #include <sys/types.h>
    #include "definitions.h"

    extern pid_t lowPriorityQueue[maxProcesses];
    extern int nextInLow;
    extern pid_t highPriorityQueue[maxProcesses];
    extern int nextInHigh;

    extern pid_t nextProcess;

    void addToQueue(pid_t process, int priority);
    void removeFromQueue(pid_t process);
    pid_t nextProcessToschedule();

#endif