#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "process.h"
#include "shmem.h"
#include "semaphore.h"
#include "manager.h"

#define FILE_NAME       "log.txt"

/*Lista (dipla sundedemenh) pou anaparista tin katanomh tis mnhmhs tou memory management*/
typedef struct memory_segment {
    process_t *process;
    unsigned int segment_size;
    struct memory_segment *next;
    struct memory_segment *previous;
}memory_segment_t;

/*Lista anamonhs twn process*/
typedef struct process_node {
    process_t process;
    struct process_node *next;
}process_node_t;

typedef struct thread_node {
    pthread_t thread;
    struct thread_node *next;
}thread_node_t;

/*Lista pou krataei statistika gia kena mnhmhs*/
typedef struct empty_node {
    unsigned int empty;
    unsigned int duration;
    struct empty_node *next;
}empty_node_t;


void* paratash(void*);
int best_fit_add(process_t);
int worst_fit_add(process_t);
int best_worst_fit_remove(process_t process);
int buddy_add(process_t process);
int buddy_add_recursion(process_t process, memory_segment_t *segment);
int buddy_remove(process_t process);
int buddy_remove_recursion(process_t process, unsigned int start, unsigned int end);
int add_wait_list(process_t process);
int empty_memory(void);
void show_memory(void);
void show_wait_list(void);
int swap_list_to_memory(time_t now);


memory_segment_t *memory = NULL;
process_node_t *l = NULL;
thread_node_t *nhmata = NULL;
empty_node_t *empty_list = NULL;


/*deites stis antistoixes sunartiseis*/
int (*add_process)(process_t) = NULL;
int (*remove_process)(process_t) = NULL;
unsigned int memory_size;
FILE *fp;
int ginomeno_xronou_mnhmhs = 0;
time_t wra_allaghs = 0;


/*S: megethos mnhmhs*/
void manager(unsigned int S, unsigned int D, int algorithm)
{
    char flag;
    process_t process, *new_process;
    thread_node_t *new_node;
    empty_node_t *new_empty, *empty_node;
    unsigned int empty;
    float average_empty, diakumansh = 0.0f;
    int duration;
    
    
    /*Arxikopoihsh domhs mnhmhs*/
    if((memory = (memory_segment_t *) malloc (sizeof(memory_segment_t))) == NULL)
    {
        perror("Den boresa na arxikopoihsw tin domh pou anaparista tin mnhmh");
        return;
    }
    
    if((fp = fopen(FILE_NAME, "w")) == NULL)
    {
        fprintf(stderr, "Den boresa na anoixw to arxeio %s: %s\n", FILE_NAME, strerror(errno));
        return;
    }
    
    switch(algorithm) {
        case BEST_FIT:
            add_process = best_fit_add;
            remove_process = best_worst_fit_remove;
            break;
        case WORST_FIT:
            add_process = worst_fit_add;
            remove_process = best_worst_fit_remove;
            break;
        case BUDDY:
            add_process = buddy_add;
            remove_process = buddy_remove;
            break;
    }
    
    memory->process = NULL;
    memory->segment_size = S;
    memory->next = NULL;
    memory->previous = NULL;
    
    memory_size = S;
    
    show_memory();
    show_wait_list();
    
    while(1)
    {
        if(semaphore_down(MESSAGE) == -1)
        {
            perror("Den boresa na katevasw ton shmaforo MESSAGE");
            break;
        }

        /*flag: vp_start 'h vp_stop 'h manager_stop*/
        read_message(&flag, &process);

        /*eidopoiei ton manager (M) gia na diavasei to mhnuma*/
        if(semaphore_up(MEMORY) == -1)
        {
            perror("Den boresa na anevasw ton shmaforo MEMORY");
            /*gia na borei na sunexisei na diavazei kainourio munhma*/
            semaphore_up(MESSAGE);
            break;
        }

        switch(flag)
        {
        case VP_START:
            
            /*An h wra_allaghs den exei arxikopoihthei tote 
             einai h prwth fora pou erxetai diergasia*/
            if(wra_allaghs == 0)
                wra_allaghs = process.start_time;
            
            /*Gia na exasfalistei amoivaios apokleismos stin mnhmh kai stin
             oura anamonhs apo ton manager pou paei na valei diergasies kai apo
             ta nhmata pou ftiaxnei gia na vgaloun diergasies*/
            if(semaphore_down(PROCESSES) == -1)
            {
                perror("Den boresa na katevasw ton shmaforo PROCESSES");
                break;
            }
            
            /*metrhsh kenwn mnhmhs prin na ginei allagh (na bei h diergasia sth mnhmh)*/
            empty = empty_memory();
            
            switch(add_process(process))
            {
            case -1:
                perror("Den boresa na topothetisw diergasia stin mnhmh");
                /*gia na boresei o epomenos na valei process stin mnhmh*/
                semaphore_up(PROCESSES);
                break;
            case 0: /*h diergasia den xwraei stin mnhmh*/
                
                printf("M: H diergasia %d me duration %d kai memory size %d den xwraei sth mnhmh\n",
                            process.process_id, process.duration, process.memory_size);
                if(add_wait_list(process) == -1)
                {
                    perror("Den boresa na topothetisw diergasia stin lista");
                    semaphore_up(PROCESSES);
                    break;
                }
                
                show_memory();
                show_wait_list();
                
                if(semaphore_up(PROCESSES) == -1)
                {
                    perror("Den boresa na anevasw ton shmaforo PROCESSES");
                    break;
                }
                break;
            case 1: /*bhke diergasia sth mnhmh*/
                
                if((new_empty = (empty_node_t *) malloc (sizeof(empty_node_t))) == NULL)
                    perror("Den boresa na arxikopoihsw ton komvo gia ta statistika");
                
                new_empty->empty = empty;
                duration = process.start_time - wra_allaghs; /*egine allagh*/
                if (duration < 0)
                    duration = 0;
                new_empty->duration = duration;
                new_empty->next = empty_list;
                empty_list = new_empty;
                
                /*enhmerwsh ths wras allaghs gia tin epomenh allagh pou tha ginei*/
                wra_allaghs = process.start_time;
                
                printf("M: H diergasia %d me duration %d sec kai memory size %d kb mphke sth mnhmh meta apo anamonh 0 sec thn xronikh stigmh %s",
                            process.process_id, process.duration, process.memory_size, ctime(&(process.start_time)));
                fprintf(fp, "M: H diergasia %d me duration %d sec kai memory size %d kb mphke sth mnhmh meta apo anamonh 0 sec thn xronikh stigmh %s",
                            process.process_id, process.duration, process.memory_size, ctime(&(process.start_time)));
                show_memory();
                show_wait_list();
                
                if(semaphore_up(PROCESSES) == -1)
                {
                    perror("Den boresa na anevasw ton shmaforo PROCESSES");
                    break;
                }
                
                break;
            }      
            break;
        case VP_STOP:
            printf("M: Mou zhthsan na stamatisw mia diergasia me id: %d\n", process.process_id);
            
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
            *new_process = process;
            /*dhmiourgia thread gia stamathma tis diergasias meta apo sugkekrimenh paratash*/
            if( pthread_create(&(new_node->thread), NULL, paratash, new_process) != 0 )
            {
                perror("Den borese na xekinhsei nhma gia stop");
                free(new_node);
                break;
            }
            new_node->next = nhmata;
            nhmata = new_node;

            break;
        case MANAGER_STOP:
            fprintf(fp, "To Ginomeno xronou mnhmhs pou vrethike einai: %d sta %d (%f%%)\n", 
                    ginomeno_xronou_mnhmhs, S * D, ginomeno_xronou_mnhmhs * 100.0f / (D * S));
            
            average_empty = (D*S - ginomeno_xronou_mnhmhs) / (float)D;
            for(empty_node = empty_list; empty_node != NULL; empty_node = new_empty)
            {   
                diakumansh += empty_node->duration * (empty_node->empty - average_empty) * (empty_node->empty - average_empty);
                new_empty = empty_node->next;
                free(empty_node);
            }
            
            diakumansh = diakumansh/D;
            
            fprintf(fp, "H mesh timh tou megethous twn kenwn mnhmhs einai %f kai h diakumansh %f\n", average_empty,diakumansh);
            
            if(fclose(fp) == EOF)
                fprintf(stderr, "Den boresa na kleisw to arxeio %s: %s\n", FILE_NAME, strerror(errno));
            
            return;
        }
    }
    
}

void* paratash(void* process)
{
    process_t *p = (process_t *) process;
    time_t end_time;
    unsigned int empty;
    empty_node_t *new_empty;
    int duration;

    /*Sunxronismos gia tin diagrafh twn processes*/
    if(semaphore_down(PROCESSES) == -1)
    {
        perror("Den boresa na katevasw ton shmaforo PROCESSES");
        return NULL;
    }
    
    empty = empty_memory();
    
    /*diagrafh diergasias apo tin mnhmh*/
    if(remove_process(*p))
    {
        
        if((new_empty = (empty_node_t *) malloc (sizeof(empty_node_t))) == NULL)
            perror("Den boresa na arxikopoihsw ton komvo gia ta statistika");

        new_empty->empty = empty;
        
                            /*pote teleiwse h diergasia - pote egine h prohgoumeh allagh*/
        duration = p->start_time + p->duration - wra_allaghs; /*egine allagh*/
        if (duration < 0)
            duration = 0;
        new_empty->duration = duration;
        new_empty->next = empty_list;
        empty_list = new_empty;
        
        wra_allaghs = p->start_time + p->duration;
        
        /*kathe fora pou teleiwnei mia diergasia upologizetai posh mnhmh
         kai gia posh diarkeia exei xrhsimopoihsei*/
        ginomeno_xronou_mnhmhs += p->duration * p->memory_size;
        
        end_time = p->start_time + p->duration;
        printf("M: H diergasia %d me duration %d sec kai memory size %d kb vghke apo th mnhmh thn xronikh stigmh %s",
                p->process_id, p->duration, p->memory_size, ctime(&end_time));
        fprintf(fp, "M: H diergasia %d me duration %d sec kai memory size %d kb vghke apo th mnhmh thn xronikh stigmh %s",
                p->process_id, p->duration, p->memory_size, ctime(&end_time));

        /*afou h diergasia p xekinise sto start_time kai teleiwse meta apo duration
         teleiwse ekeinh tin xronkh stigmh*/
        /*exetazei an borei na kaleifthei to keno apo allh process pou vrisketai sthn oura anamonhs*/
        if(swap_list_to_memory(p->start_time + p->duration) == -1)
            perror("Den boresa na topothetisw diergasia apo tin lista stin mnhmh");
        show_memory();
        show_wait_list();
    }
    else
        printf("M: H diergasia %d den vrethike stin mnhmh\n", p->process_id);
    
    if(semaphore_up(PROCESSES) == -1)
    {
        perror("Den boresa na anevasw ton shmaforo PROCESSES");
        return NULL;
    }
    
    free(process);
    return NULL;
}


int best_fit_add(process_t process)
{
    memory_segment_t *segment, *best_fit = NULL, *empty;
    
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*elenxos an to kommati einai adeio kai xwraei na bei to process*/
        if( (segment->process == NULL) && (segment->segment_size >= process.memory_size) && 
                                /*elenxos an einai mikrotero kommati mnhmhs(best_fit)*/
              ((best_fit == NULL) || (segment->segment_size < best_fit->segment_size)) )
        {
            /*an einai NULL arxika to best_fit, arxikopoieitai ws segment*/
            best_fit = segment;
        }
    }
    
    /*den uparxei katallhlos xwros gia na topothetithei h diergasia*/
    if(best_fit == NULL)
    {
        return 0;
    }
    
    /*xwraei to process sto keno kommati mnhmhs akrivws kai topotheteitai*/
    if(best_fit->segment_size == process.memory_size)
    {
        /*dhmiourgeia xwrou gia to neo process*/
        if((best_fit->process = (process_t *) malloc (sizeof(process_t))) == NULL)
        {
            return -1;
        }
        
        /*antigrafh tou process orismatos sto process tou segment*/
        memcpy(best_fit -> process, &process, sizeof(process_t));
        best_fit->segment_size = process.memory_size;
    }
    else /*periseuei kommati mnhmhs, opote prepei na spasei*/
    {
        if((empty = (memory_segment_t *) malloc (sizeof(memory_segment_t))) == NULL)
        {
            return -1;
        }
        empty->process = NULL;
        /*to megethos einai to upoloipo tou best_fit me to process */
        empty->segment_size = (best_fit->segment_size - process.memory_size);
        /*prosthikh stin lista tis mnhmhs*/
        empty->next = best_fit->next;
        empty->previous = best_fit;
        
        if((best_fit->process = (process_t *) malloc (sizeof(process_t))) == NULL)
        {
            free(empty);
            return -1;
        }
        
        /*gemisma tou best fit*/
        memcpy(best_fit -> process, &process, sizeof(process_t));
        best_fit->segment_size = process.memory_size;
        
        if(best_fit->next != NULL) /*o best_fit exei epomeno*/
                best_fit->next->previous = empty;
        best_fit->next = empty;
    }
    
    return 1;
}

int worst_fit_add(process_t process)
{
    memory_segment_t *segment, *worst_fit = NULL, *empty;
    
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*elenxos an to kommati einai adeio kai xwraei na bei to process*/
        if( (segment->process == NULL) && (segment->segment_size >= process.memory_size) && 
                                /*elenxos an einai megalutero kommati mnhmhs(worst_fit)*/
              ((worst_fit == NULL) || (segment->segment_size > worst_fit->segment_size)) )
        {
            /*an einai NULL arxika to best_fit, arxikopoieitai ws segment*/
            worst_fit = segment;
        }
    }
    
    /*den uparxei katallhlos xwros gia na topothetithei h diergasia*/
    if(worst_fit == NULL)
    {
        return 0;
    }
    
    /*xwraei to process sto keno kommati mnhmhs akrivws kai topotheteitai*/
    if(worst_fit->segment_size == process.memory_size)
    {
        /*dhmiourgeia xwrou gia to neo process*/
        if((worst_fit->process = (process_t *) malloc (sizeof(process_t))) == NULL)
        {
            return -1;
        }
        
        /*antigrafh tou process orismatos sto process tou segment*/
        memcpy(worst_fit->process, &process, sizeof(process_t));
        worst_fit->segment_size = process.memory_size;
    }
    else /*periseuei kommati mnhmhs, opote prepei na spasei*/
    {
        if((empty = (memory_segment_t *) malloc (sizeof(memory_segment_t))) == NULL)
        {
            return -1;
        }
        empty->process = NULL;
        /*to megethos einai to upoloipo tou best_fit me to process */
        empty->segment_size = (worst_fit->segment_size - process.memory_size);
        /*prosthikh stin lista tis mnhmhs*/
        empty->next = worst_fit->next;
        empty->previous = worst_fit;
        
        if((worst_fit->process = (process_t *) malloc (sizeof(process_t))) == NULL)
        {
            free(empty);
            return -1;
        }
        
        /*gemisma tou best fit*/
        memcpy(worst_fit->process, &process, sizeof(process_t));
        worst_fit->segment_size = process.memory_size;
        
        if(worst_fit->next != NULL) /*o worst_fit exei epomeno*/
                worst_fit->next->previous = empty;
        worst_fit->next = empty;
    }
    
    return 1;
}

int best_worst_fit_remove(process_t process)
{
    memory_segment_t *segment, *temp;
    
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*vrethike to process id pros diagrafh*/
        if((segment->process != NULL) && (segment->process->process_id == process.process_id))
        {
            /*Anevazetai edw gia na mhn kathusteroun ta alla nhmata perimenontas auto to nhma pou koimatai*/
            if(semaphore_up(PROCESSES) == -1)
            {
                perror("Den boresa na anevasw ton shmaforo PROCESSES");
                return -1;
            }

            /*xronos paratashs apo tin wra pou lhfthhke apo ton manager mexri tin stigmh
                pou bike stin mnhmh*/
            sleep(segment->process->duration);
            
            if(semaphore_down(PROCESSES) == -1)
            {
                perror("Den boresa na anevasw ton shmaforo PROCESSES");
                return -1;
            }
            
            /*kanenas geitonas den einai adeios, apla adeiazei auto to kommati mnhmhs*/
            if((segment->previous != NULL) && (segment->previous->process != NULL) &&
                    (segment->next != NULL) && (segment->next->process != NULL))
            {
                free(segment->process);
                segment->process = NULL;
            }
            /*o epomenos komvos einai kenos 'h den uparxei, autos o komvos diagrafetai kai o xwros tou
             prostithetai ston epomeno pou einai adeios (an uparxei)*/
            else if((segment->previous != NULL) && (segment->previous->process != NULL))
            {
                /*o epomenos uparxei kai einai kenos*/
                if(segment->next != NULL)
                {
                    segment->previous->next = segment->next;
                    segment->next->previous = segment->previous;

                    free(segment->process);
                    segment->next->segment_size += segment->segment_size;
                    free(segment);
                }
                /*o epomenos den uparxei, opote to adeiazei to segment*/
                else
                {
                    free(segment->process);
                    segment->process = NULL;
                }
            }
            /*o prohgoumenos komvos einai kenos 'h den uparxei, autos o komvos diagrafetai kai o xwros
             tou prostithetai ston prohgoumeno (an uparxei) pou einai kenos*/
            else if((segment->next != NULL) && (segment->next->process != NULL))
            {
                /*o prohgoumenos uparxei kai einai kenos*/
                if(segment->previous != NULL)
                {
                    segment->previous->next = segment->next;
                    segment->next->previous = segment->previous;

                    free(segment->process);
                    segment->previous->segment_size += segment->segment_size;
                    free(segment);
                }
                /*o prohgoumenos den uparxei, opote adeiazei to segment*/
                else
                {
                    free(segment->process);
                    segment->process = NULL;
                }
            }
            /*kai ta duo geitonika kommatia einai adeia 'h den uparxoun, opote diagrafontai 
             kai o xwros tous prostithetai sto segment to opoio epishs adeiazei (an uparxoun)*/
            else 
            {
                if(segment->previous != NULL)
                {
                    temp = segment->previous;
                    segment->segment_size += temp->segment_size;
                    segment->previous = temp->previous;
                    
                    if(temp->previous != NULL)
                        temp->previous->next = segment;
                    else
                        memory = segment;

                    /*adeiazei to keno kommati mnhmhs*/
                    free(temp->process);
                    free(temp);
                }
                
                if(segment->next != NULL)
                {
                    temp = segment->next;
                    segment->segment_size += temp->segment_size;
                    segment->next = temp->next;
                    
                    if(temp->next != NULL)
                        temp->next->previous = segment;
                    
                    free(temp->process);
                    free(temp);
                }
                
                /*adiazma tou trexontos komvou*/
                free(segment->process);
                segment->process = NULL;
            }
            
            return 1;
        }
    }
    /*exei prospelasei olh tin mnhmh kai den vrike to segment pros adeiasma*/
    return 0;
}

int buddy_add(process_t process)
{
    memory_segment_t *segment;

    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*Gia na bei mesa h diergasia sthn mnhmh tha prepei na isxuei:
         2^(u-1) < Process < 2^u*/
         switch(buddy_add_recursion(process, segment))
         {
            case -1: /*egine sfalma*/
                return -1;
            case 1: /*bhke h process me epituxia sth mnhmh*/
                return 1;
            case 0: /*den xwraei ston sugkekrimeno komvo*/
                break;
         }
    }
    
    return 0;
}

int buddy_add_recursion(process_t process, memory_segment_t *segment)
{
    memory_segment_t *new_node;
    unsigned int left_segment, right_segment; 
    /*den exei process kai xwraei h diergasia*/
    if( (segment->process == NULL) && (segment->segment_size >= process.memory_size)) /* =< 2^u*/
    {
        /*megethos aristerou paidiou*/
        left_segment = segment->segment_size / 2;
        /*megethos dexiou paidiou*/
        right_segment = segment->segment_size - left_segment;

        /*den xwraei na bei oute sto dexi oute sto aristero segment*/
        if((process.memory_size > left_segment) && (process.memory_size > right_segment))
        {
            /*xwraei omws ston segment (ekei pou einai)*/
            /*afou einai NULL h process mesa sto segment, desmeuetai xwros gia 
             * na bei h nea process */
            if( (segment->process = (process_t*) malloc (sizeof(process_t))) == NULL )
            {
                perror("Den boresa na desmeusw xwro gia to neo process");
                return -1;
            }

            *(segment->process) = process;

            return 1;
        }

        if((new_node = (memory_segment_t*) malloc (sizeof(memory_segment_t))) == NULL)
        {
            perror("Den boresa na desmeusw xwro gia neo segment mnhmhs");
            return -1;
        }
        new_node->process = NULL;
        new_node->segment_size = right_segment;
        new_node->next = segment->next;
        new_node->previous = segment;

        segment->segment_size = left_segment;
        if (segment -> next != NULL)
                segment->next->previous = new_node;
        segment->next = new_node;
        
        /*xwraei na bei sto aristero paidi (segment)*/
        if(segment->segment_size >= process.memory_size)
        {
            return buddy_add_recursion(process, segment);
        }
        
        /*xwraei na bei sto dexi paidi (new_node)*/
        if(new_node->segment_size >= process.memory_size)
        {
            return buddy_add_recursion(process, segment);
        }
    }
    
    return 0;
    
}

int buddy_remove(process_t process)
{
    return buddy_remove_recursion(process, 0, memory_size);
}

int buddy_remove_recursion(process_t process, unsigned int start, unsigned int end)
{
    memory_segment_t *segment;
    unsigned int left_start, left_end, right_start, right_end, sum = 0;
    memory_segment_t *left = NULL, *right = NULL;
    int found = 0;

    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*an h arxh tou trexontos segment (sum) einai meta tin arxh tou komvou
         pou psaxnei, den uparxei o komvos sth lista (an uphrxe tha ton eixe brei hdh). 
         Omoia kai gia to telos */
        if ((sum > start) || (sum + segment->segment_size > end))
            break;
        /*to segment vrethike atofio sth lista*/
        if ((start == sum) && (end == sum + segment->segment_size))
        {
            /*to segment exei th diergasia pou theloume*/
            if((segment->process != NULL) && (segment->process->process_id == process.process_id))
            {
                
                /*Anevazetai edw gia na mhn kathusteroun ta alla nhmata perimenontas auto to nhma pou koimatai*/
                if(semaphore_up(PROCESSES) == -1)
                {
                    perror("Den boresa na anevasw ton shmaforo PROCESSES");
                    return -1;
                }

                /*xronos paratashs apo tin wra pou lhfthhke apo ton manager mexri tin stigmh
                    pou bike stin mnhmh*/
                sleep(segment->process->duration);

                if(semaphore_down(PROCESSES) == -1)
                {
                    perror("Den boresa na anevasw ton shmaforo PROCESSES");
                    return -1;
                }
                
                /*exei vrei ton komvo tis listas pou prepei na adeiasei kai ton adeiazei */
                free(segment->process);
                segment->process = NULL;
                return 1;
            }
            else
            {
                /*den sunexizei h anadromh, einai sigourh oti den uparxei*/
                return 0;
            }
        }
        sum += segment->segment_size;
    }

    /*den vrethike komvos sth lista pou na xekinaei apo start kai na teleiwnei se end, 
     opote sunexizei anadromika na meiwnei to start kai to end mexri na pesei se komvo sti lista*/
    left_start = start;
    left_end = start + (end - start)/2;
    
    right_start = left_end;
    right_end = end;
    
    /*gia to aristero kommati*/
    /*xekinaei na psaxnei an to megethos tis diergasias einai mikrotero 'h iso
     apo megethos tou komvou*/
    if(process.memory_size <= (left_end - left_start))
        found = buddy_remove_recursion(process, left_start, left_end);
    
    /*gia to dexi kommati*/
    if ((process.memory_size <= (right_end - right_start)) && (!found))
        found = buddy_remove_recursion(process, right_start, right_end);
    
    
    /*elenxos an einai kai to left kai to right tou komvou einai adeia, 
     * opote mporoun na enw8oun*/
    /*psaxnei gia to left paidi*/
    sum = 0;
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*to segment vrethike atofio sth lista - akrivws ta oria*/
        if ((left_start == sum) && (left_end == sum + segment->segment_size))
        {
            left = segment;
            break;
        }
        sum += segment->segment_size;
    }
    
    /*psaxnei gia to right paidi*/
    sum = 0;
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        /*to segment vrethike atofio sth lista*/
        if ((right_start == sum) && (right_end == sum + segment->segment_size))
        {
            right = segment;
            break;
        }
        sum += segment->segment_size;
    }
    
    /*einai adeia o dexios kai aristeros komvos*/
    if ((left != NULL) && (left->process == NULL) && (right != NULL) && (right->process == NULL))
    {
        /*enwsh twn duo komvwn*/
        left->segment_size += right->segment_size;
        left->next = right->next;
        if(right->next != NULL)
                right->next->previous = left;
        free(right);
    }
    
    return found;
}

int add_wait_list(process_t process)
{
    process_node_t *new_node, **temp;
    
    if((new_node = (process_node_t *)malloc(sizeof(process_node_t))) == NULL)
    {
        return -1;
    }
    
    new_node->process = process;
    
    /*proxwraei o deikths temp mexri na ftasei sto telos tis listas l*/
    for(temp = &l; *temp != NULL; temp = &((*temp)->next));
    
    *temp = new_node;
    new_node->next = NULL;

    return 0;
}

int swap_list_to_memory(time_t now)
{
    process_node_t **process_temp, *temp;
    int delay;
    
    /*skanarisma tis listas l kai prosthikh */
    for(process_temp = &l; *process_temp != NULL; process_temp = &((*process_temp)->next))
    {
        /*kathisterish pou perimene h diergasia apo tin stigmh pou 
          bike sti lista anamonhs mexri tin stigmh pou tha bei sti mnhmh (now)*/
        delay = now - ((*process_temp)->process.start_time);
        /*xronos pou bike h diergasia sth mnhmh*/
        (*process_temp)->process.start_time = now;
        switch(add_process((*process_temp)->process)) {
        case -1:
            return -1;
        case 1:
            printf("M: H diergasia %d me duration %d sec kai memory size %d kb mphke sth mnhmh meta apo anamonh %d sec thn xronikh stigmh %s",
                    (*process_temp)->process.process_id, (*process_temp)->process.duration, 
                    (*process_temp)->process.memory_size, delay, ctime(&((*process_temp)->process.start_time)));
            fprintf(fp, "M: H diergasia %d me duration %d sec kai memory size %d kb mphke sth mnhmh meta apo anamonh %d sec thn xronikh stigmh %s",
                (*process_temp)->process.process_id, (*process_temp)->process.duration, 
                (*process_temp)->process.memory_size, delay, ctime(&((*process_temp)->process.start_time)));
            /*diagrafh apo tin lista*/
            temp = *process_temp;
            *process_temp = (*process_temp)->next;
            free(temp);
            /*h diergasia bhke sth mnhmh kai afairethike apo tin lista*/
            /*yparxei pithanothta na xwraei na bei akoma mia diergasia, 
             opote xana kaleitai h idia sunartish*/
            return swap_list_to_memory(now);
        case 0:
            /*den bike stin mnhmh, opote menei stin lista opws einai*/
            break;
        }
    }
    return 0;
}

int empty_memory(void)
{
    memory_segment_t *segment;
    unsigned int empty = memory_size;
    
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        if(segment->process != NULL)
            empty -= segment->process->memory_size;
    }
    
    return empty;
}

void show_memory(void)
{
    memory_segment_t *segment;
    
    printf("\t\t\t\t=== Memory Map ===\n");
    for(segment = memory; segment != NULL; segment = segment->next)
    {
        printf("%u Kb", segment->segment_size);
        if (segment->process == NULL)
            printf(" emtpy\n");
        else
            printf(" process %u me duration %d, memory size %d, receive time %ld, start time %ld\n",
                    segment->process->process_id, segment->process->duration, segment->process->memory_size,
                    segment->process->receive_time, segment->process->start_time);
    }
    printf("\t\t\t\t=== Memory Map ===\n\n");
}

void show_wait_list(void)
{
    process_node_t *process;
    
    printf("\t\t\t\t=== Wait List ===\n");
    for(process = l; process != NULL; process = process->next)
    {
        printf("process %u me duration %d, memory size %d, receive time %ld, start time %ld\n",
                    process->process.process_id, process->process.duration, process->process.memory_size,
                    process->process.receive_time, process->process.start_time);
    }
    printf("\t\t\t\t=== Wait List ===\n\n");
}
