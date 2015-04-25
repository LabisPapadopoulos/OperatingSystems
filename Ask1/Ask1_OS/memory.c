/* 
 * File:   memory.c
 * Author: labis
 *
 */
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "memory.h"

#define PERMISSIONS 0600
    
int shm_id;
void *shmem;

int shm_init(void)
{
    /* Dhmiourgia diamoirazomenhs mnhmhs IN & OUT */
    if( (shm_id = shmget(IPC_PRIVATE, sizeof(pid_t) + sizeof(int) + SIZE * sizeof(char), IPC_CREAT | PERMISSIONS)) == -1 ) 
    {
        return -1;
    }
    
    /* Attach (xrhsh mnhmhs) tin diamoirazomenh mnhmh */
    if( (shmem = shmat(shm_id, NULL, 0)) == NULL)
    {
        /* periptwsh apotuxeias attach tou deikth, diagrafh mnhmhs */
        shmctl(shm_id, IPC_RMID, NULL);
        return -1;
    }
    
    return 0;
}

int shm_finalize(void)
{
    /* Detach thn mnhmh */
    if (shmdt(shmem) == -1) 
    {
        shmctl(shm_id, IPC_RMID, NULL);
        return -1;
    }
    
    /* Diagrafh ths mnhmhs */
    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        return -1;
    }
    
    return 0;
}

void read_request(pid_t *pid, int *num)
{
        /* to, from, megethos */
    memcpy(pid, shmem, sizeof(pid_t));
    memcpy(num, shmem + sizeof(pid_t), sizeof(int));
}

void write_request(pid_t pid, int num)
{
    /*grafei to pid kai to num sthn mnhmh*/
    memcpy(shmem, &pid, sizeof(pid_t));
    memcpy(shmem + sizeof(pid_t), &num, sizeof(int));
}

void read_response(char *text)
{
    memcpy(text, shmem + sizeof(pid_t) + sizeof(int), SIZE * sizeof(char));
}

void write_response(char *text)
{
    memcpy(shmem + sizeof(pid_t) + sizeof(int), text, SIZE * sizeof(char));
}