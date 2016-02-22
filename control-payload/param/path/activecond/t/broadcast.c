#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const char  *program;
static size_t thread;
static size_t iter;
static size_t single_load;
static size_t cs_load;
static size_t ncs_load;


static size_t ready;
static size_t done;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t single = PTHREAD_COND_INITIALIZER;
static pthread_cond_t worker = PTHREAD_COND_INITIALIZER;


static void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", program);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	
	fprintf(stderr, "\nsyntax: %s <workers> <iteration> <single-cycle> "
		"<cs-cycle> <ncs-cycle> [<pinning...>]\n", program);
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

static void *clock_work(void *arg __attribute__((unused)))
{
	size_t i;

	pthread_mutex_lock(&lock);
	for (i=0; i<iter; i++) {
		while (ready != thread)
			pthread_cond_wait(&single, &lock);
		ready = 0;

		work(single_load);

		pthread_cond_broadcast(&worker);
	}

	done = 1;
	pthread_mutex_unlock(&lock);

	return NULL;
}

static void *worker_work(void *arg __attribute__((unused)))
{
	while (1) {
		pthread_mutex_lock(&lock);
	
		ready++;
		pthread_cond_broadcast(&single);
		
		pthread_cond_wait(&worker, &lock);
		if (done)
			break;

		work(cs_load);
	
		pthread_mutex_unlock(&lock);

		work(ncs_load);
	}

	pthread_mutex_unlock(&lock);
	return NULL;
}


int main(int argc, const char **argv)
{
	pthread_t *tids;
	char *err;
	size_t i, pin;
	cpu_set_t set;
	
	program = argv[0];
	
	if (argc < 6)
		error("invalid count of argument");

	thread = strtol(argv[1], &err, 10);
	if (*err)
		error("invalid argument worker : '%s'", argv[1]);

	iter = strtol(argv[2], &err, 10);
	if (*err)
		error("invalid argument iteration : '%s'", argv[2]);

	single_load = strtol(argv[3], &err, 10);
	if (*err)
		error("invalid argument single-cycle : '%s'", argv[3]);

	cs_load = strtol(argv[4], &err, 10);
	if (*err)
		error("invalid argument cs-cycle : '%s'", argv[4]);

	ncs_load = strtol(argv[5], &err, 10);
	if (*err)
		error("invalid argument ncs-cycle : '%s'", argv[5]);


	tids = malloc(sizeof (pthread_t) * (1 + thread));
	if (tids == NULL)
		error("allocation failure");

	for (i=0; i<(1 + thread); i++) {
		if (i == 0) {
			if (pthread_create(&tids[i], NULL, clock_work, NULL))
				error("threading failure");
		} else {
			if (pthread_create(&tids[i], NULL, worker_work, NULL))
				error("threading failure");
		}

		if ((int) i + 6 < argc) {
			pin = strtol(argv[i + 6], &err, 10);
			if (*err)
				error("wrong pinning '%s'", argv[i + 6]);
			
			CPU_ZERO(&set);
			CPU_SET(pin, &set);
			pthread_setaffinity_np(tids[i], sizeof(set), &set);
		}
	}

	for (i=0; i<(1 + thread); i++)
		pthread_join(tids[i], NULL);
	free(tids);

	return EXIT_SUCCESS;
}
