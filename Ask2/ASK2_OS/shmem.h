/* 
 * File:   shmem.h
 * Author: labis
 *
 * Created on December 24, 2012, 7:06 PM
 */

#ifndef SHMEM_H
#define	SHMEM_H

#ifdef	__cplusplus
extern "C" {
#endif

/* 
 * File:   shmem.c
 * Author: labis
 *
 */

#define VP_START        0
#define VP_STOP         1
#define MANAGER_STOP    2

int memory_init();

int memory_finalize();

void read_message( char *flag, process_t *process );

void write_message( char flag, process_t process );



#ifdef	__cplusplus
}
#endif

#endif	/* SHMEM_H */

