/* 
 * File:   main.c
 * Author: labis
 *
 */
#define _XOPEN_SOURCE /*Xrhsh tis _XOPEN_SOURCE gia to ipc.h - warning kata to compile*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "semaphore.h"
#include "memory.h"

#define N               5
#define LOG_FILE        "log.txt"

int server(int, int, int, int);

int client(int, int, int, int, double);

/*
 * 
 */
int main(int argc, char** argv) {

    pid_t c;
    int sem_in, sem_out, sem_server, status, i, n = 0;
    double l = 0.0;
    
    for( i = 0; i < argc; i++ )
    {
                            /*exasfalish oti uparxei kai to epomeno tou orismatos*/
        if( (strcmp("-n", argv[i]) == 0) && (i + 1 < argc) )
        {
            n = atoi(argv[i + 1]);
        }
        else if( (strcmp("-l", argv[i]) == 0) && (i + 1 < argc) )
        {
            l = atof(argv[i + 1]);
        }
    }
    
    if( n == 0 )
    {
        fprintf(stderr, "Den vrethike egkurh timh gia n\n");
        fprintf(stderr, "USAGE: %s -n <clients> -l <lamda>\n", argv[0]);
        return -1;
    }
    
    if( l == 0.0 )
    {
        fprintf(stderr, "Den vrethike egkurh timh gia l\n");
        fprintf(stderr, "USAGE: %s -n <clients> -l <lamda>\n", argv[0]);
        return -1;
    }
    
    /* arxikopoish shmaforou gia na klhronomithei apo olous */
    /* IPC_PRIVATE: gia dhmiourgeia monadikou shmaforou kathe fora
     */
    if( (sem_in = sem_init(IPC_PRIVATE)) == -1 )
    {
        perror("Den borese na ginei h arxikopoihsh tou shmaforou gia to IN");
        return -1;
    }
    
    /*Anevasma shmaforou se 1: arxikh timh gia na ginei argotera P(down)*/
    if(sem_V(sem_in) == -1)
    {
        perror("Den boresa na anevasw ton shmaforo IN");
        sem_finalize(sem_in);
        return -1;
    }
    
    if( (sem_out = sem_init(IPC_PRIVATE)) == -1 )
    {
        perror("Den borese na ginei h arxikopoihsh tou shmaforou gia to OUT");
        sem_finalize(sem_in);
        return -1;
    }
    
    /*Anevasma shmaforou se 1: arxikh timh gia na ginei argotera P(down)*/
    if(sem_V(sem_out) == -1)
    {
        perror("Den boresa na anevasw ton shmaforo OUT");
        sem_finalize(sem_in);
        sem_finalize(sem_out);
        return -1;
    }
    
    /*Arxikopoieitai se 0 automata, opote den xreiazetai na ton peiraxoume*/
    if( (sem_server = sem_init(IPC_PRIVATE)) == -1 )
    {
        perror("Den borese na ginei h arxikopoihsh tou shmaforou gia ton SERVER");
        sem_finalize(sem_in);
        sem_finalize(sem_out);
        return -1;
    }
    
    if( shm_init() == -1 )
    {
        perror("Den borese na ginei h arxikopoihsh ths koinhs mnhmhs");
        sem_finalize(sem_in);
        sem_finalize(sem_out);
        sem_finalize(sem_server);
        return -1;
    }
    
    switch (c = fork()) {
    case 0: /* paidi - client C */
        
        return client(n, sem_in, sem_out, sem_server, l);
        
    case -1:
        perror("Apotuxeia stin dhmiourgeia tou C kai S");
        return -1;
    default: /* pateras - server S */
        
        
        server(n, sem_in, sem_out, sem_server);
        
        /* O server (S) perimenei na termatisei mono o client (C) */
        if (waitpid(c, &status, 0) == -1) {
            perror("O server (S) apetuxe na perimenei ton client (C)");
            shm_finalize();
            sem_finalize(sem_in);
            sem_finalize(sem_out);
            sem_finalize(sem_server);
            return -1;
        }
        
        if( shm_finalize() == -1 )
        {
            perror("Apotuxia stin apodesmeush ths koinhs mnhmhs");
            sem_finalize(sem_in);
            sem_finalize(sem_out);
            sem_finalize(sem_server);
            return -1;
        }
        
        if( sem_finalize(sem_in) == -1 )
        {
            perror("Apotuxia stin apodesmeush tou shmaforou IN");
            sem_finalize(sem_out);
            sem_finalize(sem_server);
            return -1;
        }
        
        if( sem_finalize(sem_out) == -1 )
        {
            perror("Apotuxia stin apodesmeush tou shmaforou OUT");
            sem_finalize(sem_server);
            return -1;
        }
        
        if( sem_finalize(sem_server) == -1 )
        {
            perror("Apotuxia stin apodesmeush tou shmaforou SERVER");
            return -1;
        }
        
    }
    return 0;
}

int server(int n, int sem_in, int sem_out, int sem_server)
{
    pid_t *server_children, client_pid;
    int i, status, client_num, chars_read, sem_client;
    FILE *fp;
    char file_name[FILENAME_MAX], text[SIZE];
    
    printf("O server S xekinise\n");
    if( (server_children = (pid_t *)malloc(n * sizeof(pid_t))) == NULL )
    {
        perror("Apotuxeia stin desmeush mnhmhs gia ta pid twn paidiwn tou server S");
        return -1;
    }
    
    for(i = 0; i < n; i++)
        server_children[i] = -1;

    /*Ta S' pou dhmiourgei o server*/
    for(i = 0; i < n; i++)
    {
        /* katevazei ton shmaforo tou gia na diavasei tin aitish*/
        if(sem_P(sem_server) == -1)
        {
            perror("O server S den borese na katevasei ton shmaforo SERVER");
            break; /*an kati paei strava, stamataei na dexetai C'*/
        }
        
        read_request(&client_pid, &client_num);

        /*anevasma tou shmaforou tis mnhmhs IN gia na borei na grapsei kapoios C'
         kainouria aithsh*/
        if(sem_V(sem_in) == -1)
        {
            perror("O server S den borese na anevasei ton shmaforo IN");
            break;
        }


        switch(server_children[i] = fork()) {
            case 0:
                printf("To paidi S' me pid: %u molis xekinise\n", getpid());
                sprintf(file_name, "./%d.txt", client_num);
                
                if( (fp = fopen(file_name, "r")) == NULL )
                {
                    fprintf(stderr, "To paidi S' me pid: %u den borese na anoixei to arxeio: %s: %s\n", getpid(), file_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                
                chars_read = fread(text, sizeof(char), SIZE-1, fp);
                text[chars_read] = '\0';
                
                if(ferror(fp)) /*elenxos an egine swsta h fread*/
                {
                    fprintf(stderr, "To paidi S' me pid: %u den borese na diavasei to arxeio: %s: %s\n", getpid(), file_name, strerror(errno));
                    exit(EXIT_FAILURE);
                }

                if(sem_P(sem_out) == -1) /*katevasma shmaforou out*/
                {
                    fprintf(stderr, "To paidi S' me pid: %u den borese na katevasei ton shmaforo OUT: %s\n", getpid(), strerror(errno));
                    exit(EXIT_FAILURE);
                }


                write_response(text);
                
                /*Dhmiourgeia dunamika enos shmaforou gia na xupnisei ton C' gia na diavasei tin apantish*/
                /*Kleidi tis dhmiourgeias tou shmaforou einai to client_pid pou egrapse o client stin aitish tou*/
                if( (sem_client = sem_init(client_pid)) == -1 ) /*Arxikopoihsh se 0 automata*/
                {
                    fprintf(stderr, "To paidi S' me pid: %u den borese na dhmiourghsei shmaforo CLIENT: %s", getpid(), strerror(errno));
                    /*Afou phge strava, anevazei ton shmaforo OUT gia na erthei kapoios allos S' na grapsei*/
                    sem_V(sem_out);
                    exit(EXIT_FAILURE);
                }
                
                /*Anavazoume ton shmaforo pou phrame pio prin gia na xupnisoume ton C' na diavasei tin apantish*/
                if(sem_V(sem_client) == -1)
                {
                    fprintf(stderr, "To paidi S' me pid: %u den borese na anevasei ton shmaforo CLIENT: %s", getpid(), strerror(errno));
                    /*Afou phge strava, anevazei ton shmaforo OUT gia na erthei kapoios allos S' na grapsei*/
                    sem_V(sem_out);
                    exit(EXIT_FAILURE);
                }
                
                printf("To paidi S' me pid: %u molis termatise\n", getpid());
                exit(EXIT_SUCCESS);
            case -1: /*sfalma stin fork*/
                perror("Provlima stin dhmiourgeia enos S'");
                break;
        }

        
    }
    
    for(i = 0; i < n; i++) /* O server S perimenei ta S' */
    {
        if( (server_children[i] != -1) && (waitpid(server_children[i], &status, 0) == -1) )
        {
            fprintf(stderr, "Den boresa na perimenw to paidi S' me pid: %d\n", server_children[i]);
        }
    }
    
    free(server_children);
    
    printf("O server S stamatise\n");
    return 0;
}

int client(int n, int sem_in, int sem_out, int sem_server, double l)
{
    int i, status, file_num, client_pid, sem_client;
    pid_t *client_children;
    char text[SIZE];
    time_t time_start, time_end;
    FILE *fp;
    double sleep_time;
    
    printf("O client C xekinise\n");
    if( (client_children = (pid_t *)malloc(n * sizeof(pid_t))) == NULL )
    {
        perror("Apotuxeia stin desmeush mnhmhs gia ta pid twn paidiwn tou client C");
        return -1;
    }
    
    for(i = 0; i < n; i++)
        client_children[i] = -1;
    
    /* srand gia ton C */
    srand(time(NULL));
    
    for(i = 0; i < n; i++)
    {
        switch(client_children[i] = fork())
        {
        case 0: /* paidi tou client (C') */
            printf("To paidi C' me pid: %u molis xekinise\n", getpid());
            
            /* srand gia kathe C', gia tuxaia arxeia */
            srand(time(NULL));
            
            file_num = rand()%N;
            
            if( sem_P(sem_in) == -1 ) /* down ton shmaforo sem_in - P(in) */
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na kanei down ton IN : %s\n", getpid(), strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            client_pid = getpid();
            
            write_request(client_pid, file_num);

            time_start = time(NULL);

            /*anevasma shmaforou SERVER gia na xupnhsei ton Server kai na diavasei tin aitish*/
            if (sem_V(sem_server) == -1)
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na anevasei ton shmaforo SERVER : %s\n", client_pid, strerror(errno));
                /*anevasma tou shmaforou in gia na prospathisei kapoios allos C' na steilei aithma ston server*/
                sem_V(sem_in);
                exit(EXIT_FAILURE);
            }
            
            /*Proswpikos Shmaforos*/
            /*Dhmiourgeia dunamika enos shmaforou gia diavasei tin apantish*/
            /*Kleidi tis dhmiourgeias tou shmaforou einai to client_pid pou egrapse o client stin aitish tou*/
            if( (sem_client = sem_init(client_pid)) == -1 ) /*Arxikopoihsh se 0 automata*/
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na dhmiourghsei shmaforo CLIENT: %s\n", client_pid, strerror(errno));
                /*Afou phge strava, anevazei ton shmaforo OUT gia na erthei kapoios allos S' na grapsei*/
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }

            /*Katevazei ton shmaforo pou phre pio prin gia na diavasei tin apantish. 
             Tha prepei na exei anevei apo ton S' prwta gia na xeblokarei kai na diavasei.*/
            if(sem_P(sem_client) == -1)
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na katevasei ton shmaforo CLIENT: %s\n", client_pid, strerror(errno));
                /*Afou phge strava, anevazei ton shmaforo OUT gia na erthei kapoios allos S' na grapsei*/
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }


            /*afou ton xrhsimopoihse gia na diavasei tin apantish, ton katharizei*/
            if(sem_finalize(sem_client) == -1)
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na katharisei ton shmaforo CLIENT: %s\n", client_pid, strerror(errno));
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }

            read_response(text);
            
            time_end = time(NULL);
            
            /* Eggrafh sto arxeio se auto to shmeio oso einai akoma kleidomenos o shmaforos sem_out gia na mhn 
             blekontai ta grapsimata twn C' sto log file. */
            
            /*a: apend, eggrafh twn log mhnumatwn sto telos*/
            if( (fp = fopen(LOG_FILE, "a")) == NULL )
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na anoixei to arxeio %s: %s\n", client_pid, LOG_FILE, strerror(errno));
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }
            
            if( fprintf(fp, "C' me pid: %u, \n%supevala aithma gia to arxeio: %d\n%sphra apantish: %s\n", 
                    client_pid, ctime(&time_start), file_num, ctime(&time_end), text) < 0 )
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na grapsei sto arxeio %s: %s\n", client_pid, LOG_FILE, strerror(errno));
                fclose(fp);
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }
            
            if(fclose(fp) == EOF)
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na kleisei to arxeio %s: %s\n", client_pid, LOG_FILE, strerror(errno));
                sem_V(sem_out);
                exit(EXIT_FAILURE);
            }
            
            /*anevasma tou shmaforou OUT gia na borei o S' na grapsei nea apantish sti mnhmh*/
            if(sem_V(sem_out) == -1)
            {
                fprintf(stderr, "To paidi C' me pid: %u den borese na anevasei ton shmaforo OUT: %s\n", client_pid, strerror(errno));
                exit(EXIT_FAILURE);
            }

            printf("To paidi C' me pid: %u molis termatise\n", getpid());
            exit(EXIT_SUCCESS);
        case -1:
            perror("Apotuxeia stin dhmiourgeia enos C'");
        }
        
        /* O client C koimatai sleep_time */
        sleep_time = -(log(rand()/(double)RAND_MAX))/l;
        
        printf("Koimamai gia %f\n", sleep_time);
        
        sleep(sleep_time);
    }
    
    /* o idios o client (C)*/
    for(i = 0; i < n; i++) /* O client C perimenei ta C' */
    {
        if( (client_children[i] != -1) && (waitpid(client_children[i], &status, 0) == -1) )
        {
            fprintf(stderr, "Den boresa na perimenw to paidi C' me pid: %d\n", client_children[i]);
        }
    }
    
    free(client_children);
    
    printf("O client C stamatise\n");
    
    return 0;
}
