#define _GNU_SOURCE

#include <getopt.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static const char *progname = PROGNAME;
static unsigned long thread_count;
static unsigned long main_loop_count;
static unsigned long delay_loop_count;
static int option_yield;


void usage(void)
{
	printf("Usage: %s [-V | --version] [-h | --help]\n", progname);
	printf("Brief description of what the tool does...\n\n");
	printf("Options:\n  -h, --help                  Print this message, "
	       "then exit.\n  -V, --version               Print the program "
	       "version, then exit.\n");
}

void version(void)
{
	printf("%s %s\n%s\n%s\n", PROGNAME, VERSION, AUTHOR, MAILTO);
}

void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", progname);
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\nPlease type '%s --help' for more informations\n",
		progname);

	exit(EXIT_FAILURE);
}


void parse_options(int *_argc, char *const **_argv)
{
	int c, idx, argc = *_argc;
	char * const*argv = *_argv;
	static struct option options[] = {
		{"help",      no_argument,       0, 'h'},
		{"version",   no_argument,       0, 'V'},
		{"yield",     no_argument,       0, 'y'},
		{ NULL,       0,                 0,  0}
	};

	opterr = 0;

	while (1) {
		c = getopt_long(argc, argv, "h" "V" "y", options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		case 'V':
			version();
			exit(EXIT_SUCCESS);
		case 'y':
			option_yield = 1;
			break;
		default:
			error("unknown option '%s'", argv[optind-1]);
		}
	}

	*_argc -= optind;
	*_argv += optind;
}

void parse_arguments(int argc, char *const *argv)
{
	char *err;

	if (argc != 3)
		error("invalid argument count");
	
	thread_count = strtol(argv[0], &err, 10);
	if (*err)
		error("invalid thread-count argument '%s'", argv[0]);
	
	main_loop_count = strtol(argv[1], &err, 10);
	if (*err)
		error("invalid main-loop-count argument '%s'", argv[1]);

	delay_loop_count = strtol(argv[2], &err, 10);
	if (*err)
		error("invalid delay-loop-count argument '%s'", argv[2]);
}


static pthread_t *tids;

static pthread_barrier_t barrier;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static unsigned long *times;


static unsigned long now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);

	return ts.tv_sec * 1000000000ul + ts.tv_nsec;
}

static void thread_job(unsigned long *slot)
{
	size_t i, j;
	unsigned long start, end;

	pthread_barrier_wait(&barrier);

	start = now();
	if (option_yield) {
		for (i=0; i<main_loop_count; i++) {
			pthread_mutex_lock(&mutex);
			pthread_yield();
			pthread_mutex_unlock(&mutex);
			
			for (j=0; j<delay_loop_count; j++)
				asm volatile ("nop" : : : "memory");
		}
	} else {
		for (i=0; i<main_loop_count; i++) {
			pthread_mutex_lock(&mutex);
			pthread_mutex_unlock(&mutex);
			
			for (j=0; j<delay_loop_count; j++)
				asm volatile ("nop" : : : "memory");
		}
	}
	end = now();

	*slot = end - start;
}

static void *thread_wrapper(void *arg)
{
	thread_job((unsigned long *) arg);
	return NULL;
}


int main(int argc, char *const *argv)
{
	size_t i;
	unsigned long sum;
	
	progname = argv[0];
	parse_options(&argc, &argv);
	parse_arguments(argc, argv);

	tids = malloc(thread_count * sizeof (pthread_t));
	if (tids == NULL)
		error("allocation error");
	
	times = malloc(thread_count * sizeof (unsigned long));
	if (times == NULL)
		error("allocation error");

	pthread_barrier_init(&barrier, NULL, thread_count + 1);

	
	for (i=0; i<thread_count; i++)
		pthread_create(&tids[i], NULL, thread_wrapper, &times[i]);

	pthread_barrier_wait(&barrier);
	
	for (i=0; i<thread_count; i++)
		pthread_join(tids[i], NULL);


	sum = 0;
	for (i=0; i<thread_count; i++)
		sum += times[i];
	printf("%lu\n",
	       (sum / thread_count) / main_loop_count);
	
	return EXIT_SUCCESS;
}
