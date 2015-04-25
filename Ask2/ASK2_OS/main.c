/* 
 * File:   main.c
 * Author: labis
 *
 */
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process.h"
#include "generator.h"
#include "manager.h"

/* D: diarkeia prosomeiwshs, 
 * t: mesos xronos metaxu duo diaforetikwn diergasiwn
 * T: mesos xronos zwhs diergasias
 * lo: katw orio gia mnhmh diergasias
 * hi: anw orio gia mnhmh diergasias
 * S: megethos mnhmhs
 */
int main(int argc, char** argv) {

    int status, i;
    unsigned int D, t, T, lo, hi, S, alg;
    
    D = 0;
    t = 0;
    T = 0;
    lo = 0;
    hi = 0;
    S = 0;
    alg = 0;
    
    for(i = 0; i < argc; i++)
    {
        if((strcmp("-D", argv[i]) == 0 ) && (i + 1 < argc))
            D = atoi(argv[i + 1]);
        else if((strcmp("-t", argv[i]) == 0 ) && (i + 1 < argc))
            t = atoi(argv[i + 1]);
        else if((strcmp("-T", argv[i]) == 0 ) && (i + 1 < argc))
            T = atoi(argv[i + 1]);
        else if((strcmp("-lo", argv[i]) == 0 ) && (i + 1 < argc))
            lo = atoi(argv[i + 1]);
        else if((strcmp("-hi", argv[i]) == 0 ) && (i + 1 < argc))
            hi = atoi(argv[i + 1]);
        else if((strcmp("-S", argv[i]) == 0 ) && (i + 1 < argc))
            S = atoi(argv[i + 1]);
        else if((strcmp("-alg", argv[i]) == 0 ) && (i + 1 < argc))
            alg = atoi(argv[i + 1]);
    }
    
    if(D == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia D\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if(t == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia t\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
            
    if(T == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia T\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if(lo == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia lo\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if(hi == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia hi\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if(S == 0)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia S\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if(alg < 1 || alg > 3)
    {
        fprintf(stderr, "Den vrethike egkurh timh gia alg\n");
        fprintf(stderr, "USAGE: %s -D <diarkeia prosomeiwshs> \n\t\t-t <mesos xronos metaxu duo diergasiwn>\
\n\t\t-T <mesos xronos zwhs diergasias> \n\t\t-lo <katw orio gia mnhmh diergasias> \n\t\t-hi <anw orio gia mnhmh diergasias>\
\n\t\t-S <megethos mnhmhs> \n\t\t-alg <1: BEST FIT, 2: WORST FIT, 3: BUDDY>\n", argv[0]);
        return -1;
    }
    
    if( memory_init() == -1 )
    {
        perror("Den boresa na arxikopoihsw tin koinh mnhmh");
        exit(EXIT_FAILURE);
    }
    
    if( semaphore_init() == -1 )
    {
        perror("Den boresa na arxikopoihsw tous shmaforous");
        memory_finalize();
        exit(EXIT_FAILURE);
    }
    
    /*Epeidh apo default einai arxikopoihmenoi me 0, ginetai up ston
     shmaforo tis mnhmhs gia na boresei na xekinisei to G kai na steilei
     ta start kai ta stop tou*/
    if( semaphore_up(MEMORY) == -1 )
    {
        perror("Den boresa na anevasw ton shmaforo MEMORY");
        memory_finalize();
        semaphore_finalize();
        exit(EXIT_FAILURE);
    }
    
    if( semaphore_up(PROCESSES) == -1 )
    {
        perror("Den boresa na anevasw ton shmaforo PROCESSES");
        memory_finalize();
        semaphore_finalize();
        exit(EXIT_FAILURE);
    }
    
    switch(fork())
    {
    case 0: /* paidi o generator G*/
        generator(D, t, T, lo, hi);
        exit(EXIT_SUCCESS);
    case -1:
        perror("Den borese na ginei h fork()");
        memory_finalize();
        semaphore_finalize();
        exit(EXIT_FAILURE);
    default: /*pateras o manager M*/
        manager(S, D, alg);
        
        if(wait(&status) == -1)
        {
            perror("Den borese na ginei h wait()");
            memory_finalize();
            semaphore_finalize();
            exit(EXIT_FAILURE);
        }
        
        if(memory_finalize() == -1)
        {
            perror("Den boresa na diagrapsw tin koinh mnhmh");
            semaphore_finalize();
            exit(EXIT_FAILURE);
        }
        
        if(semaphore_finalize() == -1)
        {
            perror("Den boresa na diagrapsw tous shmaforous");
            exit(EXIT_FAILURE);
        }
        
        
        exit(EXIT_SUCCESS);
    }
    return (EXIT_SUCCESS);
}

