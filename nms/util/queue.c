
/*
 * queue.c
 */

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "debug.h"
#include "list.h"
#include "queue.h"

struct queue {
	struct list_head list;
	void *msg;
};

enum {
	QUEUE_SIZE=255
};

void clear_queue(struct queue_head *qhead, struct list_head *nhead)
{
	struct list_head *l = 0;

	__ndebug("into clear_queue()\n");
	pthread_mutex_lock(&qhead->lock);

	do {
		assert(!list_empty((&qhead->list)));
		l = qhead->list.next;
		list_del_init(l);
		assert(list_empty(l));
		sem_post(&qhead->producer);
		__ndebug("clear post sem\n");
		list_add_tail(l, nhead);
	} while (0 == sem_trywait(&qhead->consumer));
	assert(list_empty((&qhead->list)));

	pthread_mutex_unlock(&qhead->lock);
	__ndebug("out of clear_queue()\n");
}

void *get_msg(struct list_head *nhead)
{
	struct queue *e = 0;
	void *p = 0;
	
	e = list_entry(nhead->next, struct queue, list);
	list_del_init(&e->list);
	p = e->msg;
	assert(p);
	free(e);
	
	return p;
}
		
void __insert_queue(struct queue_head *qhead, void *p)
{
	struct queue *e = 0;
	
#ifndef __SEND_TRUE__ 
	assert(0);
#endif
	
	e = malloc(sizeof(struct queue));
	if (0 == e) {
#ifdef __SEND_TRUE__
#endif
		return;
	}

	sem_wait(&qhead->producer);

	__ndebug("into insert_queue()\n");
	pthread_mutex_lock(&qhead->lock);
	INIT_LIST_HEAD(&e->list);
	e->msg = p;
	list_add(&e->list, &qhead->list);
	pthread_mutex_unlock(&qhead->lock);
	__ndebug("out of insert_queue()\n");
	
	sem_post(&qhead->consumer);
}

void __insert_queue_tail(struct queue_head *qhead, void *p)
{
	struct queue *e = 0;
	
#ifndef __SEND_TRUE__ 
	assert(0);
#endif
	
	e = malloc(sizeof(struct queue));
	if (0 == e) {
#ifdef __SEND_TRUE__
#endif
		return;
	}

	sem_wait(&qhead->producer);

	__ndebug("into insert_queue_tail()\n");
	pthread_mutex_lock(&qhead->lock);
	INIT_LIST_HEAD(&e->list);
	e->msg = p;
	list_add_tail(&e->list, &qhead->list);
	sem_post(&qhead->consumer);
	pthread_mutex_unlock(&qhead->lock);
	__ndebug("insert post sem\n");
	__ndebug("out of insert_queue_tail()\n");
}

void init_queue(struct queue_head *qhead)
{
	pthread_mutex_init(&qhead->lock, 0);

	//pthread_mutex_lock(&qhead->lock);

	assert(QUEUE_SIZE < (SEM_VALUE_MAX));
	INIT_LIST_HEAD(&qhead->list);
	sem_init(&qhead->producer, 0, QUEUE_SIZE);
	sem_init(&qhead->consumer, 0, 0);
	assert(list_empty(&qhead->list));

	//pthread_mutex_unlock(&qhead->lock);
}

/*
 * end of queue.c
 */

