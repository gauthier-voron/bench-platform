#include <active.h>
#include <pthread.h>
#include <spinqueue.h>


static inline int __cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	struct queue *cqueue = get_cond_queue(cond);
	struct queue *mqueue = get_mutex_queue(mutex);

	self.data = mqueue;

	queue_acquire(cqueue);
	enqueue(cqueue, &self);
	queue_release(cqueue);

	pthread_mutex_unlock(mutex);
	queue_cell_wait(&self);

	return 0;
}

static inline int __cond_timedwait(pthread_cond_t *cond,
				   pthread_mutex_t *mutex,
				   struct timespec *abstime)
{
	struct queue *cqueue = get_cond_queue(cond);
	struct queue *mqueue = get_mutex_queue(mutex);
	int timeout;

	self.data = mqueue;

	queue_acquire(cqueue);
	enqueue(cqueue, &self);
	queue_release(cqueue);

	pthread_mutex_unlock(mutex);
	
	timeout = queue_cell_timedwait(&self, abstime);
	if (timeout) {
		queue_acquire(cqueue);
		dequeue(cqueue, &self);
		queue_release(cqueue);

		pthread_mutex_lock(mutex);
	}

	return 0;
}


static inline void __cond_broadcast_transfert(struct queue *mqueue,
					      struct queue_cell *head,
					      struct queue_cell *tail)
{
	struct queue_cell *temp;
	int exit, *owned;

	owned = (int *) &mqueue->data;
	queue_acquire(mqueue);

	if (__sync_bool_compare_and_swap(owned, 0, 1)) {
		temp = head;
		head = head->next;
		exit = (temp == tail);

		queue_cell_wake(temp);
	}

	if (!exit)
		enqueues(mqueue, head, tail);

	queue_release(mqueue);
}

static inline int __cond_broadcast(pthread_cond_t *cond)
{
	struct queue *cqueue = get_cond_queue(cond);
	struct queue_cell *cell, *head, *tail;
	struct queue *mqueue = NULL;

	queue_acquire(cqueue);

	cell = cqueue->head;
	while (cell != NULL) {
		if (cell->data == mqueue)
			goto next;

		if (mqueue != NULL)
			__cond_broadcast_transfert(mqueue, head, tail);

		mqueue = cell->data;
		head = cell;
	next:
		tail = cell;
		cell = cell->next;
	}

	if (mqueue != NULL)
		__cond_broadcast_transfert(mqueue, head, tail);

	queue_flush(cqueue);
	queue_release(cqueue);

	return 0;
}

static inline int __cond_signal(pthread_cond_t *cond)
{
	struct queue *cqueue = get_cond_queue(cond);
	struct queue_cell *cell;
	struct queue *mqueue;
	int *owned;

	queue_acquire(cqueue);

	cell = cqueue->head;
	if (cell != NULL) {
		dequeue(cqueue, cell);
		mqueue = cell->data;
		owned = (int *) &mqueue->data;

		queue_acquire(mqueue);

		if (__sync_bool_compare_and_swap(owned, 0, 1)) {
			queue_cell_wake(cell);
		} else {
			enqueue(mqueue, cell);
		}

		queue_release(mqueue);
	}

	queue_release(cqueue);

	return 0;
}

static inline int __cond_destroy(pthread_cond_t *cond __attribute__((unused)))
{
	return 0;
}



#define EXPORT_SYMBOL(func, symb, vers, tag, ret, proto, args)		\
	ret __##symb##_##tag proto					\
	{								\
		return func args;					\
	}								\
									\
	asm (".symver __" #symb "_" #tag ", " #symb vers)


EXPORT_SYMBOL(__cond_wait, pthread_cond_wait, "@GLIBC_2.2.5", 225,
	      int, (pthread_cond_t *cond, pthread_mutex_t *mutex),
	      (cond, mutex));
EXPORT_SYMBOL(__cond_wait, pthread_cond_wait, "@@GLIBC_2.3.2", 232,
	      int, (pthread_cond_t *cond, pthread_mutex_t *mutex),
	      (cond, mutex));


EXPORT_SYMBOL(__cond_timedwait, pthread_cond_timedwait, "@GLIBC_2.2.5", 225,
	      int, (pthread_cond_t *cond, pthread_mutex_t *mutex,
		    struct timespec *abstime),
	      (cond, mutex, abstime));
EXPORT_SYMBOL(__cond_timedwait, pthread_cond_timedwait, "@@GLIBC_2.3.2", 232,
	      int, (pthread_cond_t *cond, pthread_mutex_t *mutex,
		    struct timespec *abstime),
	      (cond, mutex, abstime));


EXPORT_SYMBOL(__cond_broadcast, pthread_cond_broadcast, "@GLIBC_2.2.5", 225,
	      int, (pthread_cond_t *cond), (cond));
EXPORT_SYMBOL(__cond_broadcast, pthread_cond_broadcast, "@@GLIBC_2.3.2", 232,
	      int, (pthread_cond_t *cond), (cond));


EXPORT_SYMBOL(__cond_signal, pthread_cond_signal, "@GLIBC_2.2.5", 225,
	      int, (pthread_cond_t *cond), (cond));
EXPORT_SYMBOL(__cond_signal, pthread_cond_signal, "@@GLIBC_2.3.2", 232,
	      int, (pthread_cond_t *cond), (cond));


EXPORT_SYMBOL(__cond_destroy, pthread_cond_destroy, "@GLIBC_2.2.5", 225,
	      int, (pthread_cond_t *cond), (cond));
EXPORT_SYMBOL(__cond_destroy, pthread_cond_destroy, "@@GLIBC_2.3.2", 232,
	      int, (pthread_cond_t *cond), (cond));
