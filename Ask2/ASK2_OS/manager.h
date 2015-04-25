/* 
 * File:   manager.h
 * Author: labis
 *
 * Created on December 24, 2012, 7:55 PM
 */

#ifndef MANAGER_H
#define	MANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "shmem.h"
#include "semaphore.h"
#include "process.h"

#define BEST_FIT        1
#define WORST_FIT       2
#define BUDDY           3
    
void manager(unsigned int, unsigned int, int);


#ifdef	__cplusplus
}
#endif

#endif	/* MANAGER_H */

