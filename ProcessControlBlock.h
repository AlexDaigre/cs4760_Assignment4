#ifndef TREE_H
#define TREE_H

struct ProcessControlBlock {
    pid_t pid;
    int totalCpuTimeUsed[2];
    int totalTimeInSystem[2];
    int timeUsedDurringLastBurst[2];
    int priority;
};

#endif