#define _GNU_SOURCE

#include <pin.h>

#include <ctype.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>


static size_t      next_mask;
static size_t      total_masks;
static cpu_set_t  *all_masks;


static size_t count_words(const char *arg)
{
	size_t count = 0;

	while (*arg != '\0') {
		if (isspace(*arg)) {
			arg++;
			continue;
		}

		count++;
		while (*arg != '\0' && !isspace(*arg))
			arg++;
	}

	return count;
}

static const char *next_word(const char *str, const char **word)
{
	const char *wrd;

	if (word == NULL)
		word = &wrd;
	
	while (isspace(*str))
		str++;
	
	if (*str == '\0') {
		*word = NULL;
		return str;
	}

	*word = str;
	while (*str != '\0' && !isspace(*str))
		str++;
	
	return str;
}


static int parse_cpumask(cpu_set_t *dest, const char *word, size_t len)
{
	char *buffer = alloca(len + 1);
	long i, start, end, swap;
	char *ptr;

	memcpy(buffer, word, len);
	buffer[len] = '\0';
	
	CPU_ZERO(dest);

	while (1) {
		start = strtol(buffer, &ptr, 10);
		buffer = ptr;

		if (*buffer == '-') {
			buffer++;
			end = strtol(buffer, &ptr, 10);
			buffer = ptr;
		} else {
			end = start;
		}

		if (start > end) {
			swap = start;
			start = end;
			end = swap;
		}

		for (i=start; i <= end; i++)
			CPU_SET(i, dest);
		
		if (*buffer != ',')
			break;
		buffer++;
	}

	if (*buffer != '\0')
		return -1;
	return 0;
}


static void acquire_round_robin(const char *arg, const char *argname)
{
	size_t count = 0, total = count_words(arg);
	cpu_set_t *masks = malloc(sizeof (cpu_set_t) * total);
	const char *word;
	int err;

	if (masks == NULL)
		errorp("failed to parse '%s' = '%s'", argname, arg);

	arg = next_word(arg, &word);
	while (word != NULL) {
		err = parse_cpumask(masks + count, word, arg - word);
		if (err != 0)
			error("failed to parse '%s' = '%s'", argname, arg);
		
		arg = next_word(arg, &word);
		count++;
	}

	free(all_masks);
	
	next_mask = 0;
	all_masks = masks;
	total_masks = total;
}


void acquire_arguments(void)
{
	char *arg;

	arg = getenv("PIN_RR");
	if (arg != NULL) {
		acquire_round_robin(arg, "PIN_RR");
		return;
	}

	error("cannot find parameters");
}
	
const cpu_set_t *get_next_cpumask(void)
{
	size_t id = __sync_fetch_and_add(&next_mask, 1);
	size_t old, new;

	do {
		old = next_mask;
		if (old < total_masks)
			break;
		new = old % total_masks;
	} while (__sync_bool_compare_and_swap(&next_mask, old, new));

	if (id >= total_masks)
		id = id % total_masks;

	return all_masks + id;
}
