/* 
 * File:   memory.h
 * Author: labis
 *
 */

#ifndef MEMORY_H
#define	MEMORY_H

#ifdef	__cplusplus
extern "C" {
#endif

#define SIZE 80
    
int shm_init(void);
int shm_finalize(void);
void read_request(pid_t *, int *);
void write_request(pid_t, int );
void read_response(char *);
void write_response(char *);

#ifdef	__cplusplus
}
#endif

#endif	/* MEMORY_H */

