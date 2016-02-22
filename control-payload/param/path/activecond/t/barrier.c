#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


static const char  *program;
static size_t       iter;
static size_t       worksize;
static size_t       groups;
static size_t       distance;


static void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, "%s: ", program);
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
	
	fprintf(stderr, "\nsyntax: %s <iter> <work> <groups> <distance>\n",
		program);
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

static void do_work(void)
{
	size_t i;
	size_t id, cnt;
	size_t loop, remain = 0;

	for (i=0; i<iter; i++) {
		cnt = 0;
#pragma omp parallel private(id, loop, remain)
		{
#pragma omp critical
			{
				id = cnt++;
			}
			loop = (worksize / 2) + ((id / groups) * distance);
			if (loop < worksize)
				remain = worksize - loop;
			
			work(loop);
#pragma omp barrier
			work(remain);
		}
	}
}


int main(int argc, const char **argv)
{
	char *err;
	
	program = argv[0];
	
	if (argc < 5)
		error("invalid count of argument");

	iter = strtol(argv[1], &err, 10);
	if (*err)
		error("invalid argument iter : '%s'", argv[1]);

	worksize = strtol(argv[2], &err, 10);
	if (*err)
		error("invalid argument work : '%s'", argv[2]);

	groups = strtol(argv[3], &err, 10);
	if (*err)
		error("invalid argument groups : '%s'", argv[3]);
	
	distance = strtol(argv[4], &err, 10);
	if (*err)
		error("invalid argument distance : '%s'", argv[4]);

	do_work();

	return EXIT_SUCCESS;
}
