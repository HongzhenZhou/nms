
/*
 * queue.h
 */

#ifndef _ZHZ_QUEUE_H_
#define _ZHZ_QUEUE_H_

struct queue_head {
	struct list_head list;
	sem_t producer;
	sem_t consumer;
	int sfd; // socket which send messages
	char dept[10];
	char type[10];
	pthread_mutex_t lock;
};

void init_queue(struct queue_head *qhead);

/*
 * Dequeue one (void *) from the queue,
 * you must remember to free the (void *).
 */
//void *unlink_queue(struct queue_head *qhead, unsigned int *len);

/* 
 * Dequeue all stuff from the queue, and move them
 * to a new queue, the new queue head -- (struct list_head *nhead)
 * must have nothing in it, in other words, it's new(static or malloc).
 *
 * You can get one (void *) once from the new queue use 
 * the fast_unlink() below.
 */
void clear_queue(struct queue_head *qhead, struct list_head *nhead);

/*
 * Get one (void *) from the temporary queue.
 * 
 * The get_msg() MUST be called in ONE thread for one 'nhead',
 * because the 'nhead' is NOT protected from reentrant!
 */
void *get_msg(struct list_head *nhead);

/*
 * You should call the macros defined below, 
 * not use direct the two internal __xx functions!!!
 */
void __insert_queue(struct queue_head *qhead, void *p);
void __insert_queue_tail(struct queue_head *qhead, void *p);

/* 
 * Yes, call us, we'll do everything good for you.
 *
 * IMPORTANT:
 *     If you won't use the macros, 
 *     remember not to touch anymore about the p pointer 
 *     after the __xx function return!
 */
#define insert_queue(qhead, p) if (1) {\
	__insert_queue(qhead, p);\
	p = 0;\
} else

#define insert_queue_tail(qhead, p) if (1) {\
	__insert_queue_tail(qhead, p);\
	p = 0;\
} else

#endif /* _ZHZ_QUEUE_H_ */

/*
 * end of queue.h
 */

