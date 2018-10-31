#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

    #include <semaphore.h>

    #define SEMNAME "/daigreSem432098786"
    #define SHMNAME "/tmp/daigreTmp23568"
    #define SHMPCBNAME "/tmp/daigreTmp96767"

    #define CLOCKVAR 0
    #define MSGVAR 1
    #define PCBVAR 0

    int msgShmId;
    int PCBShmId;
    int clockShmId;

    int* setupSharedClock();
    int* setupMsgCenter();
    struct ProcessControlBlock* setupSharedPCBs();
    sem_t* setupSemaphore();

#endif