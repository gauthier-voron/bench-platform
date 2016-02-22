#include <active.h>
#include <errno.h>
#include <pthread.h>
#include <spinqueue.h>


int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	struct queue *queue = get_mutex_queue(mutex);
	int *owned = (int *) &queue->data;

	if (__sync_bool_compare_and_swap(owned, 0, 1))
		goto out;

	queue_acquire(queue);
	if (!__sync_bool_compare_and_swap(owned, 0, 1))
		enqueue(queue, &self);
	queue_release(queue);

	queue_cell_wait(&self);

 out:
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	struct queue *queue = get_mutex_queue(mutex);
	int *owned = (int *) &queue->data;

	if (__sync_bool_compare_and_swap(owned, 0, 1))
		return 0;

	return EBUSY;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	struct queue *queue = get_mutex_queue(mutex);
	int *owned = (int *) &queue->data;
	struct queue_cell *cell;

	queue_acquire(queue);

	cell = queue->head;
	
	if (cell == NULL) {
		*owned = 0;
	} else {
		dequeue(queue, cell);
		queue_cell_wake(cell);
	}

	queue_release(queue);

	return 0;
}
