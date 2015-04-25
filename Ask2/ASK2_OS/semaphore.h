/* 
 * File:   semaphore.h
 * Author: labis
 *
 * Created on December 24, 2012, 7:08 PM
 */

#ifndef SEMAPHORE_H
#define	SEMAPHORE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MESSAGE     0
#define MEMORY      1
#define PROCESSES   2

int semaphore_init();

int semaphore_finalize();

int semaphore_up( unsigned short int sem );

int semaphore_down( unsigned short int sem );


#ifdef	__cplusplus
}
#endif

#endif	/* SEMAPHORE_H */

