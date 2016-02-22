#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>


#define SECOND       (1000000000ul)
#define WORK_TIME    (10 * SECOND)


static unsigned long gettime()
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * SECOND + ts.tv_nsec;
}

static void *work(void *arg)
{
	cpu_set_t *mask = (cpu_set_t *) arg;
	unsigned long start = gettime();
	int core;

	CPU_ZERO(mask);

	while (gettime() < start + WORK_TIME) {
		core = sched_getcpu();
		CPU_SET(core, mask);
	}

	return NULL;
}


static size_t parse_arguments(int argc, const char **argv)
{
	char *ptr;
	size_t count;
	
	if (argc < 2)
		return 0;
	if (argc > 2) {
		fprintf(stderr, "pthread: unexpected parameter '%s'\n"
			"Syntax: pthread <thread_count>\n", argv[2]);
		abort();
	}

	count = strtol(argv[1], &ptr, 10);
	
	if (*ptr != '\0') {
		fprintf(stderr, "pthread: invalid parameter '%s'\n"
			"Syntax: pthread <thread_count>\n", argv[1]);
		abort();
	}

	return count;
}


static void display_mask(const cpu_set_t *mask)
{
	char cores;
	size_t i, j;
	int display = 0;

	for (i = sizeof (*mask) - 1; i < sizeof (*mask); i--) {
		cores = 0;
		for (j=4; j<8; j++)
			if (CPU_ISSET(8 * i + j, mask))
				cores |= (1 << (j - 4));
		
		if (cores != 0)
			display = 1;
		if (display)
			printf("%x", cores);

		cores = 0;
		for (j=0; j<4; j++)
			if (CPU_ISSET(8 * i + j, mask))
				cores |= (1 << j);
		
		if (cores != 0)
			display = 1;
		if (display)
			printf("%x", cores);
	}

	printf("\n");
}

int main(int argc, const char **argv)
{
	size_t i, count = parse_arguments(argc, argv);
	pthread_t *tids = malloc(sizeof (pthread_t) * (1 + count));
	cpu_set_t *masks = malloc(sizeof (cpu_set_t) * (1 + count));

	for (i=0; i<count; i++)
		pthread_create(&tids[i], NULL, work, masks + i);

	work(masks + i);

	for (i=0; i<count; i++)
		pthread_join(tids[i], NULL);

	display_mask(masks + count);
	for (i=0; i<count; i++)
		display_mask(masks + i);
	
	return EXIT_SUCCESS;
}
