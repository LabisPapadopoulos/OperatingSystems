/* 
 * File:   semaphore.h
 * Author: labis
 *
 */

#ifndef SEMAPHORE_H
#define	SEMAPHORE_H

#ifdef	__cplusplus
extern "C" {
#endif

int sem_init(key_t);
int sem_P(int);
int sem_V(int);
int sem_finalize(int);


#ifdef	__cplusplus
}
#endif

#endif	/* SEMAPHORE_H */

