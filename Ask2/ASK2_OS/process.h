/* 
 * File:   process.h
 * Author: labis
 *
 */

#ifndef PROCESS_H
#define	PROCESS_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct {
    unsigned int process_id;
    unsigned int duration;
    unsigned int memory_size;
    time_t receive_time; /*xronos pou elave o Manager thn diergasia*/
    time_t start_time; /*xronos pou benei sth mnhmh*/
} process_t;


#ifdef	__cplusplus
}
#endif

#endif	/* PROCESS_H */

