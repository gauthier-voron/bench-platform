#define _GNU_SOURCE

#include <getopt.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static const char *progname = PROGNAME;


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
		default:
			error("unknown option '%s'", argv[optind-1]);
		}
	}

	*_argc -= optind;
	*_argv += optind;
}

void parse_arguments(int argc, char *const *argv)
{
	if (argc) {}
	if (argv) {}
}


static unsigned long now(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);

	return ts.tv_sec * 1000000000ul + ts.tv_nsec;
}


static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *thread_waiter(void *arg)
{
	int *num = (int *) arg;
	
	pthread_mutex_lock(&lock);
	pthread_cond_wait(&cond, &lock);

	printf("%lu :: %d wake up\n", now(), *num);

	pthread_mutex_unlock(&lock);

	return NULL;
}


int main(int argc, char *const *argv)
{
	int nums[10];
	pthread_t tid;
	int i;
	
	progname = argv[0];
	parse_options(&argc, &argv);
	parse_arguments(argc, argv);

	for (i=0; i<10; i++) {
		nums[i] = i;
		pthread_create(&tid, NULL, thread_waiter, &nums[i]);
	}

	printf("%lu :: start\n", now());
	sleep(1);
	
	pthread_mutex_lock(&lock);
	sleep(1);
	pthread_cond_signal(&cond);
	printf("%lu :: signal done\n", now());
	pthread_mutex_unlock(&lock);

	sleep(1);

	pthread_mutex_lock(&lock);
	pthread_cond_signal(&cond);
	printf("%lu :: signal done\n", now());
	sleep(1);
	pthread_cond_signal(&cond);
	printf("%lu :: signal done\n", now());
	sleep(1);
	pthread_mutex_unlock(&lock);
	
	sleep(1);

	pthread_mutex_lock(&lock);
	pthread_cond_broadcast(&cond);
	printf("%lu :: broadcast done\n", now());
	sleep(1);
	pthread_mutex_unlock(&lock);

	sleep(1);

	return EXIT_SUCCESS;
}
