#define _XOPEN_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include "process.h"
#include "semaphore.h"
#include "shmem.h"

typedef struct thread_node {
    pthread_t thread;
    struct thread_node * next;
}thread_node_t;

void* stopper(void* process );

thread_node_t * threads = NULL;

/* D: diarkeia prosomeiwshs, 
 * t: mesos xronos metaxu duo diaforetikwn diergasiwn
 * T: mesos xronos zwhs diergasias, 
 * lo: katw orio gia mnhmh diergasias
 * hi: anw orio gia mnhmh diergasias */
void generator(unsigned int D, unsigned int t, unsigned int T, unsigned int lo, unsigned int hi)
{
    time_t start = time(NULL), now;
    process_t *new_process;
    thread_node_t *new_node, *temp;
    unsigned int process_id = 0;
    int sleep_time = 0;
    
    srand(start);
    
    while(1)
    {
        
        now = time(NULL);
        
        if(now - start > D)
            break;
        
        if( (new_node = (thread_node_t *) malloc (sizeof(thread_node_t))) == NULL )
        {
            perror("Den boresa na dhmiourghsw neo komvo listas gia ta nhmata");
            break;
        }
        
        if( (new_process = (process_t *) malloc (sizeof(process_t))) == NULL )
        {
            perror("Den boresa na dhmiourghsw neo process");
            free(new_node);
            break;
        }
        
        new_process->process_id = process_id++;
        
        sleep_time = (-log(((float)rand())/RAND_MAX))*T;
        
        if(sleep_time < 0)
            sleep_time = 0;
        
        new_process->duration = sleep_time; /*ekthetikh katanomh*/
        new_process->memory_size = lo + ((float)rand())/RAND_MAX * (hi - lo); /*omoiomorfh katanomh metaxu lo kai hi*/
        new_process->start_time = now;
        
        if(semaphore_down(MEMORY) == -1)
        {
            perror("Den boresa na katevasw ton shmaforo MEMORY");
            free(new_node);
            free(new_process);
            break;
        }
        
        write_message(VP_START, *new_process);
        
        /*eidopoiei ton manager (M) gia na diavasei to mhnuma*/
        if(semaphore_up(MESSAGE) == -1)
        {
            perror("Den boresa na anevasw ton shmaforo MESSAGE");
            /*gia na boroun toulaxiston oi stoppers na sunexisoun*/
            semaphore_up(MEMORY);
            free(new_node);
            free(new_process);
            break;
        }
        
        printf("G: Eftiaxa tin diergasia %d me duration %d kai memory size %d\n", new_process->process_id, new_process->duration, new_process->memory_size);
        
        /*dhmiourgia thread gia stamathma tis diergasias*/
        if( pthread_create(&(new_node->thread), NULL, stopper, new_process) != 0 )
        {
            perror("Den borese na xekinhsei nhma gia stop");
            free(new_node);
            free(new_process);
            break;
        }
        new_node->next = threads;
        threads = new_node;
        
        /*katanomh Poisson*/
        sleep_time = (-log(((float)rand())/RAND_MAX)) * t;
        
        if(sleep_time < 0)
            sleep_time = 0;
        
        sleep(sleep_time);
    }
    
    while(threads != NULL)
    {
        temp = threads->next;
        if( pthread_join(threads->thread, NULL) != 0 )
        {
            fprintf(stderr, "Den boresa na kanw join to stop nhma me thread_id %ld: %s\n",threads->thread, strerror(errno) );
        }
        free(threads);
        threads = temp;
    }

    if(semaphore_down(MEMORY) == -1)
    {
        perror("Den boresa na katevasw ton shmaforo MEMORY");
    }

    write_message(MANAGER_STOP, *new_process);

    /*eidopoiei ton manager (M) gia na diavasei to mhnuma*/
    if(semaphore_up(MESSAGE) == -1)
    {
        perror("Den boresa na anevasw ton shmaforo MESSAGE");
    }
}

void* stopper(void* process)
{
    process_t *p = ((process_t *) process);
    sleep(p->duration);
    
    if(semaphore_down(MEMORY) == -1)
    {
        perror("Den boresa na katevasw ton shmaforo MEMORY");
        free(p);
        return NULL;
    }

    write_message(VP_STOP, *p);

    /*eidopoiei ton manager (M) gia na diavasei to mhnuma*/
    if(semaphore_up(MESSAGE) == -1)
    {
        perror("Den boresa na anevasw ton shmaforo MESSAGE");
        /*gia na boroun toulaxiston o generator kai oi alloi stoppers na sunexisoun*/
        semaphore_up(MEMORY);
        free(p);
        return NULL;
    }
    
    free(p);
    return NULL;
}
