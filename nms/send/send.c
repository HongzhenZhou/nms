
//
// send.c
//

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <asm/types.h>
#include <linux/icmp.h>

#ifdef __SEND_TRUE__ 
#endif

#include "debug.h"
#include "list.h"
#include "queue.h"
#include "probe.h"
#include "send.h"

#ifdef __SEND_TRUE__
#endif
	
void * do_send(void *arg)
{
#ifdef __ZHZ_DEBUG__
	int i;
#endif
	struct queue_head qhead;
	struct thr_send *ts = arg;
#ifdef __SEND_TRUE__
#endif
	
	__ndebug("into do_send()\n");
	memset(&qhead, 0, sizeof qhead);
	init_queue(&qhead);
#ifdef __SEND_TRUE__
#endif
	ts->qhead = &qhead;
#if 0
	memcpy(qhead.dept, ts->dept, DEPT_LEN);
	memcpy(qhead.type, ts->type, TYPE_LEN);
#endif

	sem_post(&sem_send);
	
	for (;;) {
		__ndebug("do_send -- into sem_wait()\n");
		sem_wait(&qhead.consumer);
#ifdef __ZHZ_DEBUG__
		sem_getvalue(&qhead.consumer, &i);
		__ndebug("do_send -- consumer == %d\n" _ i);
		sem_getvalue(&qhead.producer, &i);
		__ndebug("do_send -- producer == %d\n" _ i);
		__ndebug("do_send -- out of sem_wait()\n");
#endif
#ifdef __SEND_TRUE__
#endif
	}

	return 0;
}

//
// end of send.c
//

