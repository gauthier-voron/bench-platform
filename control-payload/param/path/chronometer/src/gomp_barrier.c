#define _GNU_SOURCE

#include <chronometer.h>
#include <override.h>


static __thread unsigned long barrier_lastcall;


OVERRIDE_DEFAULT(GOMP_1.0, 10, void, GOMP_barrier, (), (void))
{
	unsigned long start, end;

	start = rdtsc();
	ORIGINAL_VERSION(10, GOMP_barrier)();
	end = rdtsc();

	__thread_local_statistics->gomp_barrier_count++;
	__thread_local_statistics->gomp_barrier_intime
		+= end - start;
	__thread_local_statistics->gomp_barrier_outtime
		+= !!barrier_lastcall * (start - barrier_lastcall);
	
	barrier_lastcall = end;
}
