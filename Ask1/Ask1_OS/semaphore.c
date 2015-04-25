/* 
 * File:   semaphore.c
 * Author: labis
 *
 */ 
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "semaphore.h"

#define PERMISSIONS 0600

/* praxeis pou ginontai panw ston shmaforo */
union semun 
{
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

/* Arxikopoihsh shmaforwn */
int sem_init(key_t key)
{  
    /* Dhgmiourgeia shmaforou */
                                /* array me 1 stoixeio*/
    return semget(key, 1, IPC_CREAT | PERMISSIONS);
}

/* Katevasma P - down shmaforou me xrhsh tis semop */
int sem_P(int sem_id)
{
    struct sembuf semdata;
    
    /* noumero shmaforou */
    semdata.sem_num = 0; /*enas einai o shmaforos*/
    semdata.sem_op = -1; /* down o shmaforos */
    semdata.sem_flg = 0;
    
    /* praxh katavasmatos shmaforou */
    if( semop(sem_id, &semdata, 1) == -1 ) {
        return -1;
    }
    
    return 0;
}

/* Anevasma V - UP shmaforou me xrhsh tis semop */
int sem_V(int sem_id)
{
    struct sembuf semdata;
    
    semdata.sem_num = 0;
    semdata.sem_op = 1; /* up o shmaforos */
    semdata.sem_flg = 0;
    
    /* praxh anevasmatos shmaforou */
    if( semop(sem_id, &semdata, 1) == -1 ) {
        return -1;
    }
    
    return 0;
}

/* Katharismos shmaforwn */
int sem_finalize(int sem_id)
{
    return semctl(sem_id, 0, IPC_RMID, 0);
}