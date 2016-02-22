#define _GNU_SOURCE

#include <pin.h>

#include <dlfcn.h>
#include <pthread.h>


static int (*__pthread_create)(pthread_t *thread, const pthread_attr_t *attr,
			       void *(*start_routine)(void *), void *arg);


static inline void load_functions(void)
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
	const cpu_set_t *set = get_next_cpumask();

	ret = original_create(thread, attr, start_routine, arg);
	pthread_setaffinity_np(*thread, sizeof (*set), set);

	return ret;
}


static void __attribute__((constructor)) init(void)
{
	const cpu_set_t *set;
	
	acquire_arguments();
	load_functions();

	set = get_next_cpumask();
	pthread_setaffinity_np(pthread_self(), sizeof (*set), set);
}
