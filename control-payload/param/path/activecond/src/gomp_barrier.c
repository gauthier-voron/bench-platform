#include <active.h>
#include <omp.h>
#include <spinqueue.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TEAM  65536
static struct queue team_barriers[MAX_TEAM];


static inline struct queue *get_team_queue(int team_num)
{
	if (team_num >= MAX_TEAM) {
		fprintf(stderr, "activecond: error: no more space\n");
		exit(EXIT_FAILURE);
	}

	return &team_barriers[team_num];
}


void __GOMP_barrier_10(void)
{
	/* struct queue *queue = get_team_queue(omp_get_team_num()); */
	struct queue *queue = get_team_queue(0);
	struct queue_cell *cell;
	int amount = omp_get_num_threads();
	int *waiting = (int *) &queue->data;
	int nwait;

	queue_acquire(queue);

	nwait = *waiting + 1;
	if (nwait < amount) {
		*waiting = nwait;
		enqueue(queue, &self);
		
		queue_release(queue);
		queue_cell_wait(&self);
	} else {
		*waiting = 0;

		cell = queue->head;
		while (cell) {
			queue_cell_wake(cell);
			cell = cell->next;
		}

		queue_flush(queue);
		queue_release(queue);
	}
}

asm (".symver __GOMP_barrier_10, GOMP_barrier@@GOMP_1.0");
