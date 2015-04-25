/* 
 * File:   shmem.c
 * Author: labis
 *
 */
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include "process.h"

#define PERMISSIONS     0600

int shmid = -1;
void *mem = NULL;

int memory_init()
{
                                        /*VP_START 'h VP_STOP 'h MANAGER_STOP + process*/
    if( (shmid = shmget(IPC_PRIVATE, sizeof(char) + sizeof(process_t), IPC_CREAT | PERMISSIONS)) == -1 )
        return -1;
    
    /*attach tin mnhmh sto mem*/
    if( (mem = shmat(shmid, NULL, 0)) == (void *)-1)
        return -1;
    
    return 0;
}

int memory_finalize()
{
    if(mem != NULL)
    {
        /*detach to mem*/
        if(shmdt(mem) == -1)
            return -1;
    }
    
    if(shmid != -1)
    {
        /*remove tin mnhmh pou desmeutike me tin shmget*/
        if(shmctl(shmid, IPC_RMID, NULL) == -1)
            return -1;
    }
    
    return 0;
}

void read_message(char *flag, process_t *process)
{
    memcpy( flag, mem, sizeof(char) );
    memcpy( process, mem + sizeof(char), sizeof(process_t) );
}

void write_message(char flag, process_t process)
{
    memcpy( mem, &flag, sizeof(char) );
    memcpy( mem + sizeof(char), &process, sizeof(process_t) );
}
