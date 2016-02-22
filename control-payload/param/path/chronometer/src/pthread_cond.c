#define _GNU_SOURCE

#include <chronometer.h>
#include <override.h>
#include <pthread.h>
#include <stdio.h>


static __thread unsigned long cond_wait_lastcall;
static __thread unsigned long cond_broadcast_lastcall;
static __thread unsigned long cond_signal_lastcall;


static inline int __unify_pthread_cond_wait(pthread_cond_t *cond,
					    pthread_mutex_t *mutex,
					    int (*original)(pthread_cond_t *,
							    pthread_mutex_t *))
{
	unsigned long start, end;
	int ret;

	start = rdtsc();
	ret = original(cond, mutex);
	end = rdtsc();

	__thread_local_statistics->pthread_cond_wait_count++;
	__thread_local_statistics->pthread_cond_wait_intime
		+= end - start;
	__thread_local_statistics->pthread_cond_wait_outtime
		+= !!cond_wait_lastcall * (start - cond_wait_lastcall);
	
	cond_wait_lastcall = end;

	return ret;
}


OVERRIDE_VERSION(GLIBC_2.2.5, 225, int, pthread_cond_wait, (cond, mutex),
		 (pthread_cond_t *cond, pthread_mutex_t *mutex))
{
	return __unify_pthread_cond_wait
		(cond, mutex, ORIGINAL_VERSION(225, pthread_cond_wait));
}

OVERRIDE_DEFAULT(GLIBC_2.3.2, 232, int, pthread_cond_wait, (cond, mutex),
		 (pthread_cond_t *cond, pthread_mutex_t *mutex))
{
	return __unify_pthread_cond_wait
		(cond, mutex, ORIGINAL_VERSION(232, pthread_cond_wait));
}


static inline int
__unify_pthread_cond_broadcast(pthread_cond_t *cond,
			       int (*original)(pthread_cond_t *))
{
	unsigned long start, end;
	int ret;

	start = rdtsc();
	ret = original(cond);
	end = rdtsc();

	__thread_local_statistics->pthread_cond_broadcast_count++;
	__thread_local_statistics->pthread_cond_broadcast_intime
		+= end - start;
	__thread_local_statistics->pthread_cond_broadcast_outtime
		+= (!!cond_broadcast_lastcall)
		* (start - cond_broadcast_lastcall);
	
	cond_broadcast_lastcall = end;

	return ret;
}

OVERRIDE_VERSION(GLIBC_2.2.5, 225, int, pthread_cond_broadcast, (cond),
		 (pthread_cond_t *cond))
{
	return __unify_pthread_cond_broadcast
		(cond, ORIGINAL_VERSION(225, pthread_cond_broadcast));
}

OVERRIDE_DEFAULT(GLIBC_2.3.2, 232, int, pthread_cond_broadcast, (cond),
		 (pthread_cond_t *cond))
{
	return __unify_pthread_cond_broadcast
		(cond, ORIGINAL_VERSION(232, pthread_cond_broadcast));
}


static inline int
__unify_pthread_cond_signal(pthread_cond_t *cond,
			       int (*original)(pthread_cond_t *))
{
	unsigned long start, end;
	int ret;

	start = rdtsc();
	ret = original(cond);
	end = rdtsc();

	__thread_local_statistics->pthread_cond_signal_count++;
	__thread_local_statistics->pthread_cond_signal_intime
		+= end - start;
	__thread_local_statistics->pthread_cond_signal_outtime
		+= (!!cond_signal_lastcall)
		* (start - cond_signal_lastcall);
	
	cond_signal_lastcall = end;

	return ret;
}

OVERRIDE_VERSION(GLIBC_2.2.5, 225, int, pthread_cond_signal, (cond),
		 (pthread_cond_t *cond))
{
	return __unify_pthread_cond_signal
		(cond, ORIGINAL_VERSION(225, pthread_cond_signal));
}

OVERRIDE_DEFAULT(GLIBC_2.3.2, 232, int, pthread_cond_signal, (cond),
		 (pthread_cond_t *cond))
{
	return __unify_pthread_cond_signal
		(cond, ORIGINAL_VERSION(232, pthread_cond_signal));
}
