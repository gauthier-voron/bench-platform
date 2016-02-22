#include <pin.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ERRMSG_HEADER  "libpin: "
#define ERRMSG_FOOTER  "Please type 'man libpin' for more informations\n"


void warning(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, ERRMSG_HEADER);
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\n" ERRMSG_FOOTER);
}

void error(const char *format, ...)
{
	va_list ap;

	fprintf(stderr, ERRMSG_HEADER);
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, "\n" ERRMSG_FOOTER);
	abort();
}

void errorp(const char *format, ...)
{
	va_list ap;
	int errnum = errno;

	fprintf(stderr, ERRMSG_HEADER);
	
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);

	fprintf(stderr, ": %s\n" ERRMSG_FOOTER, strerror(errnum));
	abort();
}
