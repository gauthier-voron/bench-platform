#ifndef ACTIVE_H
#define ACTIVE_H


#include <pthread.h>
#include <spinqueue.h>


extern __thread struct queue_cell self;


static inline struct queue *get_cond_queue(pthread_cond_t *cond)
{
	return ((struct queue *) cond);
}

static inline struct queue *get_mutex_queue(pthread_mutex_t *mutex)
{
	return ((struct queue *) mutex);
}


#endif
