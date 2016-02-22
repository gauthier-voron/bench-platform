#ifndef SPINQUEUE_H
#define SPINQUEUE_H

#include <time.h>


struct queue_cell
{
	volatile char        wait;
	struct queue_cell   *next;
	struct queue_cell   *prev;
	void                *data;
} __attribute__((aligned(64)));

struct queue
{
	unsigned long                ticket;
	volatile unsigned long       owner;
	struct queue_cell           *head;
	struct queue_cell           *tail;
	void                        *data;
} __attribute__((aligned(64)));


/*
 * Acquire the queue, possibly waiting with spinloop.
 */
static inline void queue_acquire(struct queue *q)
{
	unsigned long t;

	t = __sync_fetch_and_add(&q->ticket, 1);

	while (q->owner != t)
		;
}

/*
 * Release the queue.
 */
static inline void queue_release(struct queue *q)
{
	asm volatile ("mfence" : : : "memory");
	q->owner++;
}


/*
 * Enqueue the queue_cell `c` at the end of the queue `q`.
 * This function assume `q` has been acquired.
 */
static inline void enqueue(struct queue *q, struct queue_cell *c)
{
	c->wait = 1;
	c->next = NULL;

	if (q->head == NULL) {
		q->head = c;
		c->prev = NULL;
	} else {
		q->tail->next = c;
		c->prev = q->tail;
	}

	q->tail = c;
}

/*
 * Enqueue the queue_cells forming a list between `h` and `t` at the end of
 * the queue `q`.
 * This function assume `q` has been acquired and cells between `h` and `t`
 * were already enqueued and not yet dequeued (i.e. waiting).
 */
static inline void enqueues(struct queue *q, struct queue_cell *h,
			    struct queue_cell *t)
{
	t->next = NULL;

	if (q->head == NULL) {
		q->head = h;
		h->prev = NULL;
	} else {
		q->tail->next = h;
		h->prev = q->tail;
	}

	q->tail = t;
}

/*
 * Dequeue the first queue_cell of the queue `q`.
 * Return the dequeued cell, or NULL if `q` is empyt.
 * This function assume `q` has been acquired.
 */
static inline void dequeue(struct queue *q, struct queue_cell *c)
{
	if (c->prev != NULL)
		c->prev->next = c->next;
	else
		q->head = c->next;

	if (c->next != NULL)
		c->next->prev = c->prev;
	else
		q->tail = c->prev;
}

/*
 * Flush the queue `q`, making it empty.
 */
static inline void queue_flush(struct queue *q)
{
	q->head = NULL;
	q->tail = NULL;
}


extern int pthread_yield();

/*
 * Wait on the queue_cell `c` until some thread call `queue_cell_wake()` on
 * this same cell.
 * The queue_cell *must* have been enqueued and not yet dequeued for wait,
 * otherwise this function does nothing.
 */
static inline void queue_cell_wait(struct queue_cell *c)
{
	volatile char *wait = &c->wait;

	while (*wait)
		;
}

/*
 * Wait on the queue_cell `c` until some thread call `queue_cell_wake()` on
 * this same cell or the current time exceed the given abstime.
 * The queue_cell *must* have been enqueued and not yet dequeued for wait,
 * otherwise this function does nothing.
 * Return 0 if it ends because of awaken by antoher thread and 1 if it ends on
 * the timeout.
 */
static inline int queue_cell_timedwait(struct queue_cell *c,
					const struct timespec *abstime)
{
	volatile char *wait = &c->wait;
	struct timespec ts;

	while (*wait) {
		clock_gettime(CLOCK_REALTIME, &ts);
		if ((ts.tv_sec  > abstime->tv_sec) ||
		    (ts.tv_sec == abstime->tv_sec &&
		     ts.tv_nsec > abstime->tv_nsec)) {
			*wait = 0;
			return 1;
		}
	}

	return 0;
}

/*
 * Wake up any thread sleeping on the queue_cell `c`.
 */
static inline void queue_cell_wake(struct queue_cell *c)
{
	c->wait = 0;
}


#endif
