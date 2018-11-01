#ifndef TREE_H
#define TREE_H

struct ProcessControlBlock {
    pid_t pid;
    int totalCpuTimeUsed;
    int timeUsedDurringLastBurst;
    int timeStarted[2];
    int priority;
};

#endif