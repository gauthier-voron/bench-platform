#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


#define DEFINE_LOCK(name) pthread_mutex_t name
#define INIT_LOCK(name) pthread_mutex_init(name, NULL)
#define TAKE_LOCK(name) pthread_mutex_lock(name)
#define RELEASE_LOCK(name) pthread_mutex_unlock(name)

/* #define DEFINE_LOCK(name) pthread_spinlock_t name */
/* #define INIT_LOCK(name) pthread_spin_init(name, PTHREAD_PROCESS_PRIVATE) */
/* #define TAKE_LOCK(name) pthread_spin_lock(name) */
/* #define RELEASE_LOCK(name) pthread_spin_unlock(name) */


static const char  *program;
static size_t       thread_count;
static size_t       thread_iter;
static size_t       thread_cs;
static size_t       thread_ncs;

static DEFINE_LOCK(lock);


static void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", program);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	
	fprintf(stderr, "\nsyntax: %s <thread-count> <thread-iteration> "
		"<thread-cs> <thread-ncs> [<pinning...>]\n", program);
	exit(EXIT_FAILURE);
}


static void work(size_t iter)
{
	unsigned int eax, edx;
	size_t i, nasty = 1;

	for (i=0; i<iter; i++) {
		if (nasty == 1) {
			asm ("rdtsc" : "=a" (eax), "=d" (edx));
			nasty = (((unsigned long) edx) << 32) | eax;
		} else if (nasty % 2) {
			nasty = nasty * 3 + 1;
		} else {
			nasty = nasty / 2;
		}
	}
}

static void *thread_work(void *arg __attribute__((unused)))
{
	size_t i;

	for (i=0; i<thread_iter; i++) {
		TAKE_LOCK(&lock);
		work(thread_cs);
		RELEASE_LOCK(&lock);

		work(thread_ncs);
	}

	return NULL;
}


int main(int argc, const char **argv)
{
	pthread_t *tids;
	char *err;
	size_t i, pin;
	cpu_set_t set;
	
	program = argv[0];
	
	if (argc < 5)
		error("invalid count of argument");

	thread_count = strtol(argv[1], &err, 10);
	if (*err)
		error("invalid argument thread-count : '%s'", argv[1]);

	thread_iter = strtol(argv[2], &err, 10);
	if (*err)
		error("invalid argument thread-iteration : '%s'", argv[2]);

	thread_cs = strtol(argv[3], &err, 10);
	if (*err)
		error("invalid argument thread-cs : '%s'", argv[3]);

	thread_ncs = strtol(argv[4], &err, 10);
	if (*err)
		error("invalid argument thread-ncs : '%s'", argv[4]);

	tids = malloc(sizeof (pthread_t) * thread_count);
	if (tids == NULL)
		error("allocation failure");

	INIT_LOCK(&lock);
	TAKE_LOCK(&lock);
	for (i=0; i<thread_count; i++) {
		if (pthread_create(&tids[i], NULL, thread_work, NULL) != 0)
				error("threading failure");

		if (argc - 5 < (int) i) {
			pin = strtol(argv[i + 5], &err, 10);
			if (*err)
				error("wrong pinning '%s'", argv[i + 5]);
			
			CPU_ZERO(&set);
			CPU_SET(pin, &set);
			pthread_setaffinity_np(tids[i], sizeof(set), &set);
		}
	}
	RELEASE_LOCK(&lock);

	for (i=0; i<thread_count; i++)
		pthread_join(tids[i], NULL);
	free(tids);

	return EXIT_SUCCESS;
}
