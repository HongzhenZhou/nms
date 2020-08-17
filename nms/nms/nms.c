
//
// nms.c
//

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <linux/limits.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __SEND_TRUE__
#endif

#include "debug.h"
#include "list.h"
#include "queue.h"
#include "probe.h"
#include "send.h"
#include "nms.h"
#if 0
#include "trap.h"
#endif

sem_t sem_send;
static struct probe_head phead;

static void set_daemon(int pidfd) 
{
	int fd;

	for (fd=0; fd<NR_OPEN; fd++) {
		if (pidfd != fd)
			close(fd);
	}

	if (fork())
		exit(1);
	
	if (-1 == setsid())
		exit(1);

	if (fork())
		exit(1);
	
	chdir("/");
}

static void check_once(char *pgn) 
{
	int pidfd;
	char pid[PIDLEN+1];
	sigset_t __mask__;
	struct flock lock;
	struct sigaction act;
#ifdef __ZHZ_DEBUG__
	char buf[80];
	time_t tt;
#endif

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	
	if (-1 == mkdir(D_LOCK, 755)) {
		if (EEXIST != errno)
			__eedebug("mkdir()" __GM__);
	}
			
	pidfd = open(F_LOCK_ZHZ, OFLAG, FMODE);

	if (-1 == fcntl(pidfd, F_SETLK, &lock)) {
		char buf[1024];
		strerror_r(errno, buf, sizeof buf);
		perror(buf);
		__debug("flock()" __GM__);
		__eedebug("Somebody lock the lockfile, i have no key:(\n");
	}

	snprintf(pid, sizeof pid, "%ld\n", (long)getpid());	
	if (-1 == ftruncate(pidfd, 0))
		__debug("ftruncate()" __GM__);
	if (-1 == write(pidfd, pid, strlen(pid))) 
		__debug("write()" __GM__);

	if (-1 == sigfillset(&__mask__))
		__eedebug("sigfillset()" __GM__);
	if (-1==sigdelset(&__mask__, SIGUSR1) || 
		-1==sigdelset(&__mask__, SIGINT) || 
		-1==sigdelset(&__mask__, SIGQUIT) ||
		-1==sigdelset(&__mask__, SIGUSR2) ||/*
		-1==sigdelset(&__mask__, SIGPIPE) ||*/
		-1==sigdelset(&__mask__, SIGABRT) /*||
		-1==sigdelset(&__mask__, SIGHUP)*/)
		__eedebug("sigdelset()" __GM__);
	if (-1 == pthread_sigmask(SIG_BLOCK, &__mask__, 0))
		__eedebug("pthread_sigmask()" __GM__);

#ifndef __ZHZ_DEBUG__
	set_daemon(pidfd);
#endif
}

static void add_probe(struct thr_send *ts)
{
	struct list_head *l;
	struct probe *top = 0, *ttop = 0;
	pthread_t tid;
	unsigned int i, num, j = 1, k;
	struct p2probe *p2p = 0, *tp2p = 0;
	pthread_attr_t attr;
	
	pthread_mutex_lock(&phead.lock);
	if (!list_empty(&phead.newprobes)) {
		l = phead.newprobes.next;
		list_del_init(l);
		pthread_mutex_unlock(&phead.lock);
	} else {
		if (list_empty(&phead.probes)) {
			assert(0==phead.run && 0==phead.num);
			for (l=phead.newprobes.next; l!=&phead.newprobes; l=l->next) {
				ttop = list_entry(l, struct probe, probes);
				l = l->prev;
				list_del_init(&ttop->probes);
				memset(ttop, 0, sizeof(struct probe));
				free(ttop);
				ttop = 0;
			}
			pthread_mutex_unlock(&phead.lock);
			init_probe(&phead);
		} else {
			pthread_mutex_unlock(&phead.lock);
			assert(0);
			__debug("ERROR: wrong init_probe restart!\n" __GM__);
		}
		return;
	}

	ttop = list_entry(l, struct probe, probes);
	__debug("%d.%d.%d.%d(%d.%d.%d.%d)\n" _ 
		((unsigned char *)&ttop->addr)[0] _ 
		((unsigned char *)&ttop->addr)[1] _ 
		((unsigned char *)&ttop->addr)[2] _ 
		((unsigned char *)&ttop->addr)[3] _
		(((unsigned char *)&ttop->mask)[0]) _ 
		(((unsigned char *)&ttop->mask)[1]) _ 
		(((unsigned char *)&ttop->mask)[2]) _ 
		(((unsigned char *)&ttop->mask)[3])); 
	num = ((255-((unsigned char *)&ttop->mask)[0])<<24) + 
		((255-((unsigned char *)&ttop->mask)[1])<<16) + 
		((255-((unsigned char *)&ttop->mask)[2])<<8) + 
		(255-((unsigned char *)&ttop->mask)[3]);
	if (num) {
		if (65536*2 < num)
			goto noop;
		ttop->num = --num;
	} else {
#if 0
		ttop->num = num = 1;
#endif
#if 1
		goto noop;
#endif
	}

	for (k=0; k<(num/3072+1); k++) {
		unsigned int c, taddr;
		unsigned char *p1;
		unsigned char *p2;
		unsigned char *p3;

		if (0 == (top=malloc(sizeof(struct probe))))
			__edebug("malloc()" __GM__);
		memset(top, 0, sizeof(struct probe));
		memcpy(top, ttop, sizeof(struct probe));
		INIT_LIST_HEAD(&top->probes);
			
		p1 = (unsigned char *)&top->addr;
		p2 = (unsigned char *)&top->sa;
		p3 = (unsigned char *)&taddr;
		for (c=0; c<4; c++)
			p3[c] = p1[3-c];
		taddr += k*3072;
		for (c=0; c<4; c++)
			p2[c] = p3[3-c];
		top->anum = (num/3072)==k ? (num-k*3072) : 3072;
		top->size = top->anum / 4;
		top->size = 0==top->size ? top->anum : top->size;
		__ndebug("top->anum == %d, top->size == %d\n" _ top->anum _ top->size);
		top->list = malloc(top->size * sizeof(struct list_head));
		if (0 == top->list)
			__edebug("malloc()" __GM__);
		for (i=0; i<top->size; i++) 
			INIT_LIST_HEAD(&top->list[i]);
		top->state = READY;
		assert(READY == top->state);
		top->phead = &phead;
		top->ts = ts;

		__ndebug("new probe" __GM__);
		pthread_mutex_lock(&phead.lock);
		list_add(&top->probes, &phead.probes);
		assert(!list_empty(&phead.probes));
		assert(!list_empty(&top->phead->probes));
		assert(!list_empty(&top->probes));
		if (0 == k) {
			phead.num++;
			phead.run++;
			assert(phead.num >= phead.run);
		}
		phead.anum++;
		phead.arun++;
		assert(phead.anum >= phead.arun);
		pthread_mutex_unlock(&phead.lock);
		if (0 == (tp2p=malloc(sizeof(struct p2probe))))
			__edebug("malloc()" __GM__);
		tp2p->top = top;
		tp2p->next = p2p;
		p2p = tp2p;
	}
	
	pthread_attr_init(&attr);
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) 
		__edebug("pthread_attr_setdetachstate()" __GM__);
	while (p2p) {
		tp2p = p2p->next;
		if (pthread_create(&tid, &attr, do_probe, p2p->top))
			__debug("pthread_create()" __GM__);
		free(p2p);
		p2p = tp2p;
	}
	pthread_attr_destroy(&attr);
	
#ifdef __SEND_TRUE__ 
#endif

noop:
	free(ttop);
}

static int ppc(char *buf, int t, char *cmd, char *val)
{
	char *start = buf;
	char *p, *s = 0;
	
	for (p=start; '\0'!=*p; p++) {
		if (!(('A'<=*p&&'Z'>=*p) || ('a'<=*p&&'z'>=*p) || '_'==*p)) {
			if (0 == s) 
				continue;
			memcpy(cmd, s, p-s);
			start = p;
			s = 0;
			break;
		}
		if ('A'<=*p && 'Z'>=*p)
			*p = *p + 'a' - 'A';
		if (0 == s)
			s = p;
	}

	if (buf == start)
		return 1;
			
	for (p=start; '\0'!=*p; p++) {
		if (!(('A'<=*p&&'Z'>=*p) || ('a'<=*p&&'z'>=*p) ||
			('0'<=*p&&'9'>=*p) || '.'==*p || '_'==*p)) {
			if (0 == s)
				continue;
			memcpy(val, s, p-s);
			return 0;
		}
		if (0 == s)
			s = p;
	}
	
	return 1;
}

static void read_addr(struct thr_send *ts)
{
	FILE *fp;
	char buf[257];
	char cmd[80];
	char val[257];
	unsigned int i = 0;
	
	memset(buf, 0, sizeof buf);

	__debug("begin read addr\n");
	while (0 == (fp=fopen(CF_NEINMS, "r"))) {
		__debug("not exist file\n");
		if (ENOENT == errno) {
			system(CP_NEINMS);
		}
	}

	__debug("check addr\n");
	while (fgets(buf, sizeof buf, fp)) {
		assert('\0' == buf[256]);
		memset(cmd, 0, sizeof cmd);
		memset(val, 0, sizeof val);
		if (ppc(buf, 257, cmd, val))
			continue;
		if (0 == memcmp(cmd, "dept", 4)) {
			strncpy(ts->dept, val, 9);
			ts->dept[9] = 0;
			i |= 1U;
		} else if (0 == memcmp(cmd, "type", 4)) {
			strncpy(ts->type, val, 9);
			ts->type[9] = 0;
			i |= 2U;
		} else if (0 == memcmp(cmd, "raddr", 5)) {
			strncpy(ts->raddr, val, INET_ADDRSTRLEN);
			ts->raddr[INET_ADDRSTRLEN-1] = 0;
			i |= 4U;
		} else if (0 == memcmp(cmd, "rport", 5)) {
			ts->rport = strtol(val, 0, 10);
			i |= 8U;
		}
	}
		
	fclose(fp);
	
	if (15U != i)
		__edebug("config error!\n");
}

int main(int argc, char **argv)
{
	pthread_t tid;
	struct thr_send ts1;
	pthread_attr_t attr;

	check_once(argv[0]);

	pthread_mutex_init(&phead.lock, 0);
	phead.num = phead.run = 0;
	INIT_LIST_HEAD(&phead.probes);
	INIT_LIST_HEAD(&phead.newprobes);
	INIT_LIST_HEAD(&phead.sends);
	phead.ts = 0;
	sem_init(&phead.sem, 0, 0);
	memset(&ts1, 0, sizeof ts1);
	INIT_LIST_HEAD(&ts1.list);
#if 0
	read_addr(&ts1);
#endif
	
	sem_init(&sem_send, 0, 0);
	__debug("create send\n");
	pthread_attr_init(&attr);
	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) 
		__edebug("pthread_attr_setdetachstate()" __GM__);
	if (pthread_create(&tid, &attr, do_send, &ts1)) 
		__edebug("pthread_create()" __GM__);
	__debug("out of create send\n");
	sem_wait(&sem_send);
	sem_destroy(&sem_send);
	list_add(&ts1.list, &phead.sends);
	phead.ts = phead.sends.next;
	
#if 0
	if (pthread_create(&tid, &attr, do_trap, &ts1)) 
		__edebug("pthread_create()" __GM__);
	
	if (pthread_create(&tid, &attr, do_conf, &ts1)) 
		__edebug("pthread_create()" __GM__);
#endif

	pthread_attr_destroy(&attr);

	__ndebug("init_probe()\n");
	assert(list_empty(&phead.probes));
	assert(list_empty(&phead.newprobes));
	init_probe(&phead);

#ifdef __SEND_TRUE__ 
#endif

	for (;;) {
		__ndebug("into sem_wit()" __GM__);
		sem_wait(&phead.sem);
		__ndebug("out of sem_wit()" __GM__);
		assert(phead.ts);
		assert(!list_empty(&phead.sends));
		add_probe(list_entry(phead.ts, struct thr_send, list));
#if 0
		phead.ts = phead.ts->next;
		if (phead.ts == &phead.sends)
			phead.ts = phead.sends.next;
		assert(phead.ts && phead.ts!=&phead.sends);
#endif
	}
	
	return 0;
}

//
// end of nms.c
//

