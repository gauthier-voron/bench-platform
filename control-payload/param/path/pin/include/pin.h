#ifndef PIN_H
#define PIN_H


#define _GNU_SOURCE

#include <sched.h>


void warning(const char *format, ...);

void error(const char *format, ...);

void errorp(const char *format, ...);


void acquire_arguments(void);

const cpu_set_t *get_next_cpumask(void);


#endif
