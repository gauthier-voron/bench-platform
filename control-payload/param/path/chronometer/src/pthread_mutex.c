#define _GNU_SOURCE

#include <chronometer.h>
#include <override.h>
#include <pthread.h>
#include <stdio.h>


static __thread unsigned long mutex_lock_lastcall;


OVERRIDE(int, pthread_mutex_lock, (mutex), (pthread_mutex_t *mutex))
{
	unsigned long start, end;
	int ret;

	start = rdtsc();
	ret = ORIGINAL(pthread_mutex_lock)(mutex);
	end = rdtsc();

	__thread_local_statistics->pthread_mutex_count++;
	__thread_local_statistics->pthread_mutex_intime
		+= end - start;
	__thread_local_statistics->pthread_mutex_outtime
		+= (!!mutex_lock_lastcall) * (start - mutex_lock_lastcall);
	
	mutex_lock_lastcall = end;

	return ret;
}
