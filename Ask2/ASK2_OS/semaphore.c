#define _XOPEN_SOURCE

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PERMISSIONS 0600

int sem_id = -1;

int semaphore_init()
{
    /* 1 set twn 3 shmaforwn: 
     * 1os gia eidopoiei o Generator ton manager 
     * 2os gia tin koinh mnhmh
     * 3os gia tis domes dedomenwn pou periexoun tis diergasies ston manager*/
   if( (sem_id = semget(IPC_PRIVATE, 3, IPC_CREAT | PERMISSIONS)) == -1 )
       return -1;
   
   return 0;
}

int semaphore_finalize()
{
    if(sem_id != -1)
    {
        /*0-> gia olous tous shmaforous*/
        if(semctl(sem_id, 0, IPC_RMID) == -1)
            return -1;
    }
    
    return 0;
}

int semaphore_up(unsigned short int sem)
{
    
    struct sembuf operation;
    
    operation.sem_num = sem;
    operation.sem_op = 1; /*up*/
    operation.sem_flg = 0;
    
    if(semop(sem_id, &operation, 1) == -1)
        return -1;
    
    return 0;
}

int semaphore_down(unsigned short int sem)
{
    struct sembuf operation;
    
    operation.sem_num = sem;
    operation.sem_op = -1; /*down*/
    operation.sem_flg = 0;
    
    if(semop(sem_id, &operation, 1) == -1)
        return -1;
    
    return 0;
}