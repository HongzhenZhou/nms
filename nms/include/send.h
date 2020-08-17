
//
// send.h
//

#ifndef _ZHZ_SEND_H_
#define _ZHZ_SEND_H_

struct thr_send {
	unsigned char raddr[INET_ADDRSTRLEN];
	unsigned short rport;
	char dept[DEPT_LEN];
	char type[TYPE_LEN];
	int nouse;
	struct queue_head *qhead;
	struct list_head list;
};

extern sem_t sem_send;

#ifdef __SEND_TRUE__ 
#endif

void *do_send(void *);

#endif // _ZHZ_SEND_H_

//
// end of send.h
//

