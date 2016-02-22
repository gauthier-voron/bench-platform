#define _GNU_SOURCE

#include <getopt.h>
#include <pthread.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/resource.h>


static const char *progname = PROGNAME;
static int *cores;


static void usage(void)
{
	printf("Usage: %s [-V | --version] [-h | --help]\n", progname);
	printf("       %s <cores...>\n"
	       "Keep the cpu busy. This is intended to replace the cpu idle "
	       "loop by a busy\n"
	       "loop which always keep the cpu at maximum frequency. The user "
	       "can select what\n"
	       "<cores> to keep busy. This process always tries to set itself "
	       "the minimum\n"
	       "priority.\n\n", progname);
	printf("Options:\n  -h, --help                  Print this message, "
	       "then exit.\n  -V, --version               Print the program "
	       "version, then exit.\n");
}

static void version(void)
{
	printf("%s %s\n%s\n%s\n", PROGNAME, VERSION, AUTHOR, MAILTO);
}


#define ERNO      (1 << 0)
#define ERFATAL   (1 << 1)

static void __error(char flags, const char *format, va_list ap)
{
	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, format, ap);

	if (flags & ERNO) {
		fprintf(stderr, ": ");
		perror("");
	} else {
		fprintf(stderr, "\n");
	}

	fprintf(stderr, "Please type '%s --help' for more informations\n",
		progname);

	if (flags & ERFATAL)
		exit(EXIT_FAILURE);
}

#define GEN_ERRFUNC(fname, flags)			\
	static void fname(const char *format, ...)	\
	{						\
		va_list ap;				\
							\
		va_start(ap, format);			\
		__error(flags, format, ap);		\
		va_end(ap);				\
	}
GEN_ERRFUNC(error, ERFATAL       )
GEN_ERRFUNC(errop, ERFATAL | ERNO)
GEN_ERRFUNC(warnp,           ERNO)
#undef GENERRFUNC


static void parse_options(int *_argc, char *const **_argv)
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
		c = getopt_long(argc, argv, "h" "V", options, &idx);
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

static void parse_arguments(int argc, char *const *argv)
{
	int i;
	char *err;

	if (argc == 0)
		error("missing core arguments");
	
	cores = malloc(argc * sizeof (int));
	if (cores == NULL)
		errop("cannot parse arguments");

	for (i=0; i<argc; i++) {
		cores[i] = strtol(argv[i], &err, 10);
		if (*err)
			error("invalid core argument: '%s'", argv[i]);
	}
}


static void busy(void)
{
	while (1)
		;
}

static void *busy_wrapper(void *arg __attribute__((unused)))
{
	busy();
	return NULL;
}


static void decrease_priority(void)
{
	struct sched_param sp;
	int ret;

	ret = setpriority(PRIO_PROCESS, getpid(), 19);
	if (ret)
		warnp("cannot decrease priority");

	sp.sched_priority = 0;
	ret = sched_setscheduler(getpid(), SCHED_IDLE, &sp);
	if (ret)
		warnp("cannot set idle priority");
}


int main(int argc, char *const *argv)
{
	pthread_t tid;
	cpu_set_t set;
	int i;
	
	progname = argv[0];
	parse_options(&argc, &argv);
	parse_arguments(argc, argv);

	decrease_priority();
	
	CPU_ZERO(&set);
	CPU_SET(cores[0], &set);

	tid = pthread_self();
	pthread_setaffinity_np(tid, sizeof (set), &set);

	for (i=1; i<argc; i++) {
		CPU_ZERO(&set);
		CPU_SET(cores[i], &set);
	
		pthread_create(&tid, NULL, busy_wrapper, NULL);
		pthread_setaffinity_np(tid, sizeof (set), &set);
	}

	busy();
	
	return EXIT_SUCCESS;
}
