#ifndef CHRONOMETER_H
#define CHRONOMETER_H


#include <stdlib.h>


struct statistics
{
	size_t pthread_create_date;
	size_t pthread_exit_date;
	
	size_t pthread_cond_wait_count;
	size_t pthread_cond_wait_intime;
	size_t pthread_cond_wait_outtime;

	size_t pthread_cond_broadcast_count;
	size_t pthread_cond_broadcast_intime;
	size_t pthread_cond_broadcast_outtime;

	size_t pthread_cond_signal_count;
	size_t pthread_cond_signal_intime;
	size_t pthread_cond_signal_outtime;

	size_t pthread_mutex_count;
	size_t pthread_mutex_intime;
	size_t pthread_mutex_outtime;

	size_t gomp_barrier_count;
	size_t gomp_barrier_intime;
	size_t gomp_barrier_outtime;

	struct statistics *next;
} __attribute__((aligned(64)));

extern __thread struct statistics *__thread_local_statistics;


static inline unsigned long rdtsc(void)
{
	unsigned int edx, eax;

	asm volatile ("mfence\nrdtsc\n" : "=d" (edx), "=a" (eax)
		      : : "ebx", "ecx");

	return (((unsigned long) edx) << 32) | eax;
}


#endif
