#define _GNU_SOURCE

#include <chronometer.h>
#include <override.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


static struct statistics *global_list_statistics = NULL;

static struct statistics main_thread_statistics;

static struct statistics trash_thread_statistics;

__thread struct statistics *__thread_local_statistics =
	&trash_thread_statistics;


static pthread_key_t pthread_atexit_key;

static void pthread_atexit(void *arg __attribute__((unused)))
{
	__thread_local_statistics->pthread_exit_date = rdtsc();
}


static inline void stub_thread(struct statistics *stats)
{
	struct statistics *list;
	
	memset(stats, 0, sizeof (*stats));

	pthread_key_create(&pthread_atexit_key, pthread_atexit);
	pthread_setspecific(pthread_atexit_key, &pthread_atexit_key);

	do {
		list = global_list_statistics;
		stats->next = list;
	} while (!__sync_bool_compare_and_swap
		 (&global_list_statistics, list, stats));
	
	__thread_local_statistics = stats;
	__thread_local_statistics->pthread_create_date = rdtsc();
}

struct stub_parameters
{
	void               *(*initial_routine)(void *);
	void                 *initial_parameters;
	struct statistics    *thread_statistics;
};

static void *stub_routine(void *args)
{
	struct stub_parameters params = *((struct stub_parameters *) args);

	stub_thread(params.thread_statistics);
	free(args);

	return params.initial_routine(params.initial_parameters);
}

OVERRIDE(int, pthread_create, (thread, attr, start_routine, args),
	 (pthread_t *thread,
	  const pthread_attr_t *attr,
	  void *(*start_routine)(void *),
	  void *args))
{
	struct stub_parameters *params;
	struct statistics *stats;

	stats = malloc(sizeof (struct statistics));
	params = malloc(sizeof (struct stub_parameters));

	if (stats == NULL || params == NULL) {
		fprintf(stderr, "chronometer: memory failure\n");
		exit(EXIT_FAILURE);
	}

	params->initial_routine = start_routine;
	params->initial_parameters = args;
	params->thread_statistics = stats;

	return ORIGINAL(pthread_create)(thread, attr, stub_routine, params);
}


static void program_atexit(void)
{
	char *output = getenv("CHRONOMETER_OUTPUT");
	struct statistics *cell;
	FILE *foutput = stderr;
	size_t len;
	char *path;
	FILE *tmp;
	
	if (output != NULL) {
		len = strlen(output);
		path = alloca(2 * len);
		snprintf(path, 2 * len - 1, output, getpid());

		if ((tmp = fopen(path, "a")) != NULL)
			foutput = tmp;
	}
	
	pthread_atexit(NULL);

	fprintf(foutput, "thread,"
		"mutex_lock_call,mutex_lock_intime,mutex_lock_outtime,"
		"cond_wait_call,cond_wait_intime,cond_wait_outtime,"
		"cond_broadcast_call,cond_broadcast_intime,"
		"cond_broadcast_outtime,"
		"cond_signal_call,cond_signal_intime,cond_signal_outtime,"
		"barrier_call,barrier_intime,barrier_outtime\n");
	for (cell = global_list_statistics; cell; cell = cell->next) {
		if (cell->pthread_exit_date == 0)
			cell->pthread_exit_date = rdtsc();
		
		fprintf(foutput,
			"%lu," "%lu,%lu,%lu," "%lu,%lu,%lu," "%lu,%lu,%lu,"
			"%lu,%lu,%lu," "%lu,%lu,%lu\n",
			cell->pthread_exit_date - cell->pthread_create_date,
			cell->pthread_mutex_count, cell->pthread_mutex_intime,
			cell->pthread_mutex_outtime,
			cell->pthread_cond_wait_count,
			cell->pthread_cond_wait_intime,
			cell->pthread_cond_wait_outtime,
			cell->pthread_cond_broadcast_count,
			cell->pthread_cond_broadcast_intime,
			cell->pthread_cond_broadcast_outtime,
			cell->pthread_cond_signal_count,
			cell->pthread_cond_signal_intime,
			cell->pthread_cond_signal_outtime,
			cell->gomp_barrier_count, cell->gomp_barrier_intime,
			cell->gomp_barrier_outtime);
	}

	if (foutput != stderr)
		fclose(foutput);
}

static void __attribute__((constructor)) setup_main_thread(void)
{
	atexit(program_atexit);
	stub_thread(&main_thread_statistics);
}
