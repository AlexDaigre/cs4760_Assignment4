struct ProcessControlBlock {
    unsigned int totalCpuTimeUsed[2];
    unsigned int totalTimeInSystem[2];
    unsigned int timeUsedDurringLastBurst[2];
    int priority;
};