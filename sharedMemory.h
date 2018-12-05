#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

    #include <semaphore.h>

    #define SEMNAME "/daigreSem432435467870"
    #define SHMNAME "/tmp/daigreTmp4357658"
    #define SHMPCBNAME "/tmp/daigreTmp568454533"

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