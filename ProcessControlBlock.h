#ifndef TREE_H
#define TREE_H

struct ProcessControlBlock {
    pid_t pid;
    int totalCpuTimeUsed;
    int totalTimeInSystem;
    int timeUsedDurringLastBurst;
    int priority;
};

#endif