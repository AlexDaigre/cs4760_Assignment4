#include <stdio.h>
#include <sys/types.h>
#include "queue.h"
#include "definitions.h"

pid_t lowPriorityQueue[maxProcesses] = {0};
int nextInLow = 0;
pid_t highPriorityQueue[maxProcesses] = {0};
int nextInHigh = 0;

int avalibleHighPriorityItems();

void addToQueue(pid_t process, int priority){
    int nextOpenPlace = -1;
    if (priority > 0){
        int i;
        for(i=0; i<maxProcesses; i++){
            if (lowPriorityQueue[i] == 0){
                nextOpenPlace = i;
                break;
            }
        }
        lowPriorityQueue[nextOpenPlace] = process;
        printf("Added %d to low queue at position %d\n", process, nextOpenPlace);
    } else {
        int i;
        for(i=0; i<maxProcesses; i++){
            if (highPriorityQueue[i] == 0){
                nextOpenPlace = i;
                break;
            }
        }
        highPriorityQueue[nextOpenPlace] = process;
        printf("Added %d to high queue at position %d\n", process, nextOpenPlace);
    }
}

void removeFromQueue(pid_t process){
    int closedProcess = -1;
    int i;
    for(i=0; i<maxProcesses; i++){
        if (lowPriorityQueue[i] == process){
            closedProcess = i;
            break;
        }
    }
    if (closedProcess >= 0){
        lowPriorityQueue[closedProcess] = 0;
        printf("Removed %d from low queue at position %d\n", process, closedProcess);
        return;
    }
    
    int j;
    for(j=0; j<maxProcesses; j++){
        if (highPriorityQueue[j] == process){
            closedProcess = j;
            break;
        }
    }
    if (closedProcess >= 0){
        highPriorityQueue[closedProcess] = 0;
        printf("Removed %d from high queue at position %d\n", process, closedProcess);
    }
}

pid_t nextProcessToschedule(){
    int i;
    int highPriorityAvalible = avalibleHighPriorityItems();
    for (i = 0; i < maxProcesses; i++){
        if (highPriorityAvalible > 0){
            if (highPriorityQueue[nextInHigh] != 0){    
                // printf("Scheduled %d from high queue\n", highPriorityQueue[nextInHigh]);
                nextInHigh = (nextInHigh + 1) % maxProcesses;
                return highPriorityQueue[nextInHigh];
            }
            nextInHigh = (nextInHigh + 1) % maxProcesses;
        } else {
            if (lowPriorityQueue[nextInLow] != 0){
                // printf("Scheduled %d from low queue\n", highPriorityQueue[nextInHigh]);
                nextInLow = (nextInLow + 1) % maxProcesses;
                return lowPriorityQueue[nextInLow];
            }
            nextInLow = (nextInLow + 1) % maxProcesses;
        }
    }
    // printf("No processes to schedule\n");
    return -1;
}

// pid_t nextProcessToschedule(){
//     int i = 0;
//     while (i < 18){

//     }
// }



int avalibleHighPriorityItems(){
    int i;
    for(i=0; i<maxProcesses; i++){
        if (highPriorityQueue[i] != 0){
            // printf("highPriorityQueue[%d] == %d\n", i, highPriorityQueue[i]);
            return 1;
        }
    }
    return 0;
}