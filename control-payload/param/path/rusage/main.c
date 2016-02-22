#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static const char *progname = PROGNAME;


void usage(void)
{
	printf("Usage: %s [-V | --version] [-h | --help] <cmd>\n", progname);
	printf("Display resource usage of the given command\n\n");
	printf("The displayed resources are:\n");
	printf("  UserTime          user CPU time used (seconds)\n");
	printf("  SystemTime        system CPU time used (seconds)\n");
	printf("  RealTime          wall clock CPU time used (seconds)\n");
	printf("  MaxRSS            maximum resident set size (kilobytes)\n");
	printf("  VolContSwitch     count of volutary context switch\n");
	printf("  InvolContSwitch   count of involutary context switch\n\n");
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


int check_arguments(int argc, char *const *argv)
{
	if (argc < 2)
		error("missing command");
	
	if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
		usage();
		return 1;
	}

	if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) {
		version();
		return 1;
	}

	return 0;
}

int main(int argc, char *const *argv)
{
	pid_t pid;
	struct rusage usage;
	struct timespec start, end;
	int status;

	progname = argv[0];
	if (check_arguments(argc, argv))
		return EXIT_SUCCESS;

	clock_gettime(CLOCK_REALTIME, &start);
	if ((pid = fork()) == 0) {
		execvp(argv[1], argv + 1);
		return EXIT_FAILURE;
	} else {
		waitpid(pid, &status, 0);
	}
	clock_gettime(CLOCK_REALTIME, &end);

	getrusage(RUSAGE_CHILDREN, &usage);
	printf("UserTime %lu\n", usage.ru_utime.tv_sec);
	printf("SystemTime %lu\n", usage.ru_stime.tv_sec);
	printf("RealTime %lu\n", end.tv_sec - start.tv_sec);
	printf("MaxRSS %lu\n", usage.ru_maxrss);
	printf("VolContSwitch %lu\n", usage.ru_nvcsw);
	printf("InvolContSwitch %lu\n", usage.ru_nivcsw);

	return WEXITSTATUS(status);
}
