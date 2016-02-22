#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static const char  *program;
static size_t       thread_prod;
static size_t       thread_cons;
static size_t       thread_iter;
static size_t       thread_room;
static size_t       thread_cs;
static size_t       thread_ncs;

static size_t          stock;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  cmore = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  cless = PTHREAD_COND_INITIALIZER;


static void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", program);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	
	fprintf(stderr, "\nsyntax: %s <producer> <consumer> "
		"<iteration> <queue-room> <cs-cycle> <ncs-cycle> "
		"[<producer-pinning...> -- <consumer-pinning>]\n", program);
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

static void *producer_work(void *arg __attribute__((unused)))
{
	size_t i;

	for (i=0; i<thread_iter; i++) {
		pthread_mutex_lock(&mutex);
		
		while (stock >= thread_room)
			pthread_cond_wait(&cless, &mutex);

		work(thread_cs);
		stock++;

		pthread_cond_signal(&cmore);
		pthread_mutex_unlock(&mutex);

		work(thread_ncs);
	}

	return NULL;
}

static void *consumer_work(void *arg __attribute__((unused)))
{
	while (1) {
		pthread_mutex_lock(&mutex);
		
		while (stock == 0)
			pthread_cond_wait(&cmore, &mutex);
		if (stock == (size_t) -1)
			break;

		work(thread_cs);
		stock--;

		pthread_cond_signal(&cless);
		pthread_mutex_unlock(&mutex);

		work(thread_ncs);
	}

	pthread_cond_signal(&cmore);
	pthread_mutex_unlock(&mutex);
	return NULL;
}


int main(int argc, const char **argv)
{
	pthread_t *tids;
	char *err;
	size_t i, j, pin;
	cpu_set_t set;
	
	program = argv[0];
	
	if (argc < 7)
		error("invalid count of argument");

	thread_prod = strtol(argv[1], &err, 10);
	if (*err)
		error("invalid argument producer : '%s'", argv[1]);

	thread_cons = strtol(argv[2], &err, 10);
	if (*err)
		error("invalid argument consumer : '%s'", argv[2]);

	thread_iter = strtol(argv[3], &err, 10);
	if (*err)
		error("invalid argument iteration : '%s'", argv[3]);

	thread_room = strtol(argv[4], &err, 10);
	if (*err)
		error("invalid argument queue-room : '%s'", argv[4]);

	thread_cs = strtol(argv[5], &err, 10);
	if (*err)
		error("invalid argument cs-cycle : '%s'", argv[5]);

	thread_ncs = strtol(argv[6], &err, 10);
	if (*err)
		error("invalid argument ncs-cycle : '%s'", argv[6]);

	tids = malloc(sizeof (pthread_t) * (thread_prod + thread_cons));
	if (tids == NULL)
		error("allocation failure");
	
	j = 7;

	pthread_mutex_lock(&mutex);
	for (i=0; i<thread_prod; i++) {
		if (pthread_create(&tids[i], NULL, producer_work, NULL) != 0)
			error("threading failure");

		if (j < (size_t) argc && strcmp(argv[j], "--")) {
			pin = strtol(argv[j], &err, 10);
			if (*err)
				error("wrong pinning '%s'", argv[j]);
			
			CPU_ZERO(&set);
			CPU_SET(pin, &set);
			pthread_setaffinity_np(tids[i], sizeof(set), &set);

			j++;
		}
	}

	if (j < (size_t) argc && !strcmp(argv[j], "--"))
		j++;

	for (i=thread_prod; i<thread_prod + thread_cons; i++) {
		if (pthread_create(&tids[i], NULL, consumer_work, NULL) != 0)
			error("threading failure");
		
		if (j < (size_t) argc) {
			pin = strtol(argv[j], &err, 10);
			if (*err)
				error("wrong pinning '%s'", argv[j]);
			
			CPU_ZERO(&set);
			CPU_SET(pin, &set);
			pthread_setaffinity_np(tids[i], sizeof(set), &set);

			j++;
		}
	}
	pthread_mutex_unlock(&mutex);

	for (i=0; i<thread_prod; i++)
		pthread_join(tids[i], NULL);

	pthread_mutex_lock(&mutex);
	stock = (size_t) -1;
	pthread_cond_signal(&cmore);
	pthread_mutex_unlock(&mutex);
	
	for (i=thread_prod; i<thread_prod + thread_cons; i++)
		pthread_join(tids[i], NULL);
	
	free(tids);

	return EXIT_SUCCESS;
}
