#define _GNU_SOURCE

#include <dlfcn.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>


static int     next_core;
static int    *available_cores;
static size_t  available_count;


static void initialize_cores(void)
{
	char *ptr, *str, *env;
	size_t cnt;

	env = getenv("PIN_RR");
	if (env == NULL) {
		fprintf(stderr, "pin: cannot find env variable PIN_RR\n");
		return;
	}

	cnt = 0;
	str = env;
	while (*str) {
		strtol(str, &ptr, 10);
		if (ptr == str) {
			fprintf(stderr, "pin; invalid env variable "
				"PIN_RR: '%s'\n", str);
			return;
		}
		cnt++;
		str = ptr;
		
		while (*str == ' ')
			str++;
	}

	if (cnt == 0) {
		fprintf(stderr, "pin: empty env variable PIN_RR\n");
		return;
	}

	available_cores = malloc(sizeof (int) * cnt);

	cnt = 0;
	str = env;
	while (*str) {
		available_cores[cnt++] = strtol(str, &ptr, 10);
		str = ptr;

		while (*str == ' ')
			str++;
	}

	available_count = cnt;
	next_core = available_cores[0];
}

static void pin_thread(pthread_t tid)
{
	cpu_set_t set;
	size_t old, new;
	int core;

	if (available_count == 0)
		return;

	do {
		old = next_core;
		new = old + 1;
		if (new == available_count)
			new = 0;
	} while (!__sync_bool_compare_and_swap(&next_core, old, new));

	core = available_cores[old];
	CPU_ZERO(&set);
	CPU_SET(core, &set);

	pthread_setaffinity_np(tid, sizeof(set), &set);
}


static int (*__pthread_create)(pthread_t *thread, const pthread_attr_t *attr,
			       void *(*start_routine)(void *), void *arg);

static inline void initialize_function(void)
{
	__pthread_create = dlsym(RTLD_NEXT, "pthread_create");
}

static inline int original_create(pthread_t *thread,
				  const pthread_attr_t *attr,
				  void *(*start_routine) (void *), void *arg)
{
	return __pthread_create(thread, attr, start_routine, arg);
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		   void *(*start_routine) (void *), void *arg)
{
	int ret;

	ret = original_create(thread, attr, start_routine, arg);
	pin_thread(*thread);

	return ret;
}


static void __attribute__((constructor)) init(void)
{
	initialize_cores();
	initialize_function();

	pin_thread(pthread_self());
}
