
//
// probe.c
//

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/icmp.h>
#include <stdio.h>
#include <time.h>

#ifdef __SEND_TRUE__
#endif

#include "debug.h"
#include "list.h"
#include "queue.h"
#include "probe.h"
#include "echo.h"
#include "snmp.h"
#include "send.h"
#include "lock.h"

//unsigned int powersave = 14400;
unsigned int powersave = 100;
unsigned int defrest = DEF_REST;

unsigned int del_down(struct probe *top, int force)
{
	struct up *pup;
	struct list_head *l, *prev;
	struct list_head cool;
	int i, cooln = 0;
#ifdef __SEND_TRUE__
#endif
#ifdef __ZHZ_DEBUG__
	unsigned char *p = 0;
#endif

	INIT_LIST_HEAD(&cool);
	__ndebug("into del_down\n");

	START_ROLL
	pup = list_entry(l, struct up, list);
#ifdef __ZHZ_DEBUG__
	p = (unsigned char *)(&pup->addr);
	if (top->last > pup->last) {
		__ndebug("old NOT update : top->last == %d, pup->last == %d, "
			"++++ %d.%d.%d.%d\n" _ top->last _ pup->last _ 
			p[0] _ p[1] _ p[2] _ p[3]);
	}
#endif
	
	if (get_ism()>top->last-pup->last && 0==force)
		continue;

	if (get_isd()>top->last-pup->last && 0==force) {
#ifdef __SEND_TRUE__ 
#endif
		continue;
	}
	
	prev = l->prev;
	list_del_init(l);
	list_add_tail(l, &cool);
	cooln++;
	l = prev;
	if (!list_empty(&pup->ifinfo)) {
		struct list_head *lp = pup->ifinfo.next;
		struct ifinfo *ifp = 0;
		while (lp!=&pup->ifinfo) {
			ifp = list_entry(lp, struct ifinfo, ifinfo);
			lp = lp->next;
			list_del_init(&ifp->ifinfo);
			free(ifp);
			ifp = 0;
		}
	}
	assert(list_empty(&pup->ifinfo));

	if (pup->nreq) {
		assert(pup->enr);
		assert(pup->nreq->req);
		if (pup->nreq->req) {
			free(pup->nreq->req);
			pup->nreq->req = 0;
		}
		free(pup->nreq);
		pup->nreq = 0;
		pup->enr = 0;
	} else
		assert(0 == pup->enr);
	
	if (pup->tf) {
		assert(pup->flowin);
		assert(pup->flowout);
		free(pup->flowin);
		free(pup->flowout);
		pup->flowin = 0;
		pup->flowout = 0;
		pup->efr = 0;
		pup->tf = 0;
	}
	assert(0 == pup->efr);
	assert(0 == pup->tf);
	assert(0 == pup->flowin);
	assert(0 == pup->flowout);
			
	__ndebug("del_down:delete--%d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
	top->eups--;
	assert(0 <= top->eups);
	END_ROLL
		
	if (0 == cooln) {
		assert(list_empty(&cool));
		__ndebug("just del NULL!\n");
		return 0;
	}
	
	__ndebug("Yes, del something down!\n");
#ifdef __SEND_TRUE__ 
#endif
	
	while (!list_empty(&cool)) {
		assert(cooln);
		l = cool.next;
		pup = list_entry(l, struct up, list);
		list_del_init(l);
#ifdef __SEND_TRUE__
#endif
		free(pup);
		cooln--;
	}

#ifdef __SEND_TRUE__
#endif

	assert(0 == cooln);
	__ndebug("OK, del something down!\n");
	
	__ndebug("del_down : force == %d, eups == %d\n" _ force _ top->eups);
	assert(0==force || 0==top->eups);
	__ndebug("out of del_down\n");

	return 0;
}
	
#define timeout(num1, to) \
	assert(top->ups<=top->anum);\
	__ndebug("intp timeout wait poll\n");\
	while (top->ups < num1) {\
		__ndebug("intp p poll\n");\
		n = poll(ufds, 1, to);\
\
		if (0 == n) {\
			__ndebug("timeout\n");\
			break;\
		} else {\
			recv_echo(fd, top);\
		}\
	}\
	__ndebug("out of timeout wait poll\n");\
\

static int less_ping(struct probe *top, unsigned int i, unsigned int j,
	unsigned int *a)
{
	struct list_head *l = 0;
	struct up *pup;
	unsigned int key, h;
	unsigned int b = 0;
	unsigned char *p = (unsigned char *)a, *q = 0;
       	unsigned char *k = (unsigned char *)&b;
	unsigned char *m = (unsigned char *)(&top->sa);

	for (h=0; h<4; h++)
		k[h] = m[3-h];
	b += i;
	for (h=0; h<4; h++) 
		p[h] = k[3-h];
	__ndebug("%d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);

	if (0 == j)
		return 0;

	key = get_key(a, &top->sa, top->size);

	for (l=top->list[key].next; l!=&top->list[key]; l=l->next) {
		pup = list_entry(l, struct up, list);
		if (!memcmp(&pup->addr, a, sizeof(unsigned int))) {
			assert(pup->last <= top->last);
			if (pup->last == top->last)
				return 1;
		}
	}
	
	return 0;
}

static void do_ping(int fd, struct probe *top)
{
	unsigned int i = 1, j = 0;
	int n;
	struct pollfd ufds[2];
	unsigned int a;

	top->ups = 0;
	ufds[0].fd = fd;
	ufds[0].events = POLLIN;
	ufds[1].fd = fd;
	ufds[1].events = POLLOUT;

	__ndebug("top->anum == %d\n" _ top->anum);

	for (j=0; j<4; j++) {
		i = 1;
		while (i <= top->anum) {
			__ndebug("i == %d, j == %d\n" _ i _ j);
			if (less_ping(top, i, j, &a)) {
				i++;
				continue;
			}
			n = poll(ufds, 2, 10000); 
			
			__ndebug("return from poll()\n");
			if (0 == n)
				return;
			else if (ufds[0].revents & POLLIN) {
				__ndebug("receive\n");
				recv_echo(fd, top);
				__ndebug("receive one echo \n");
			} else if (ufds[1].revents & POLLOUT) {
				__ndebug("send\n");
				send_echo(fd, top, &a, 0);
				__ndebug("send one echo \n");
				i++;
			} 

			if (0==((128-1)&i) && j<3) {
				timeout(top->anum, 100)
			}	
		}
	}
		
	timeout(top->anum, 1000*top->distant)
}

static void probe_clean(void *p)
{
	struct probe *top = p;
	struct list_head *l;
	int i, j;
	
	assert(list_empty(&top->probes));

	for (j=0; j<ALL_BIND; j++) {
		if (top->sr[j]) {
			assert(top->sr[j]->req);
			free(top->sr[j]->req);
			top->sr[j]->req = top->sr[j]->req_id = 0;
			free(top->sr[j]);
			top->sr[j] = 0;
		}
	}
	
	del_down(top, 1);

#ifdef __ZHZ_DEBUG__
	START_ROLL
	assert(0);
	END_ROLL
#endif
	free(top->list);

	top->list = 0;
	top->phead = 0;
	close(top->fdi);
	close(top->fds);
	memset(top, 0, sizeof(struct probe));
	free(top);
	top = 0;
}

#define _test_exit() if(1) {\
	pthread_cleanup_push(probe_clean, top);\
	pthread_testcancel();\
	pthread_cleanup_pop(0);\
} else

static void me2sleep(unsigned int rest, struct probe *top)
{
	unsigned int i;
	struct list_head *l = 0;
	struct probe* p = 0;
	struct p2probe *p2p = 0, *tp2p = 0;
	unsigned int becancel = 1, dead_life = 0;
	unsigned int psn = powersave / (4*defrest);
	struct probe_head *phead = 0;
#ifdef __ZHZ_DEBUG__
	unsigned int mask;
	unsigned char *mp = 0;
	char tbuf[80];
	time_t tt;
#endif

	__ndebug("psn = %d\n" _ psn);
	__ndebug("into me2sleep()\n");
	top->totals += rest;
	__ndebug("top->totals == %d\n" _ top->totals);
	__ndebug("---------------------------"
		"----------------------------\n");

#ifdef __ZHZ_DEBUG__
	memcpy(&mask, &top->addr, sizeof(unsigned int));
	mp = (unsigned char *)&mask;
	__ndebug("in me2sleep\n");
#endif

	//if (top->totals*defrest <= (powersave/4)) { 
	if (top->totals*defrest <= (8*powersave)) { 
		unsigned int hl;

		hl = defrest * (1 + top->phead->anum/20);
		__ndebug("REST sleep\n");
		sleep(rest * hl);
		return;
	}

	__debug("totals = %d\n" _ top->totals);
	if ((top->totals-psn+1)*powersave <= 60*powersave) {
		top->state = POWERSAVE;
		__ndebug("DEEP sleep\n");
		sleep(powersave);
		return;
	}

	if ((top->totals-psn+1)*powersave <= 200*powersave) {
		top->state = POWERSAVE;
		__ndebug("DEEP2 sleep\n");
		sleep(2 * powersave);
		return;
	}

	del_down(top, 1);
#ifdef __ZHZ_DEBUG__
	START_ROLL
	assert(0);
	END_ROLL
#endif

	if (DEAD == top->state) {
		int i;
		unsigned st = powersave / 12;
		
		__debug("DEAD sleep\n");
		_test_exit();
		for (i=0; i<24; i++) {
			sleep(st);
			_test_exit();
		}
		return;
	}

	top->state = DEAD;
	_test_exit();

	pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, &top->phead->lock);
	pthread_mutex_lock(&top->phead->lock);

	__ndebug(__GM__);
	for (l=top->phead->probes.next; l!=&top->phead->probes; l=l->next) {
		__ndebug(__GM__);
		p = list_entry(l, struct probe, probes);
		if (memcmp(&p->addr, &top->addr, sizeof(unsigned int)) || 
			memcmp(&p->mask, &top->mask, sizeof(unsigned int))) {
			__ndebug(__GM__);
			continue;
		}
		if (0==memcmp(&p->sa, &top->sa, sizeof(unsigned int)) && 
			p->pid==top->pid && 
			0==memcmp(&p->thd, &top->thd, sizeof(pthread_t))) {
			becancel = 0;
			__ndebug(__GM__);
			continue;
		}
		if (DEAD!=p->state) {
			dead_life = 1;
			__ndebug(__GM__);
			goto DEAD_LIFE;
		}
		__ndebug(__GM__);
		if (0 == (tp2p=malloc(sizeof(struct p2probe))))
			__edebug("malloc()" __GM__);
		tp2p->next = p2p;
		tp2p->top = p;
		p2p = tp2p;
	}

	if (becancel)
		goto DEAD_LIFE;

	__ndebug(__GM__);
	while (p2p) {
		tp2p = p2p->next;
		assert(p2p->top);

		list_del_init(&p2p->top->probes);
		p2p->top->phead = 0;
		top->phead->anum--;
		top->phead->arun--;
		assert(top->phead->anum >= top->phead->arun);

		pthread_cancel(p2p->top->thd);
		free(p2p);
		p2p = tp2p;
	}

	__ndebug(__GM__);
	list_del_init(&top->probes);
	assert(list_empty(&top->probes));
	top->phead->anum--;
	top->phead->arun--;
	assert(top->phead->anum >= top->phead->arun);
	top->phead->num--;
	top->phead->run--;
	assert(top->phead->num >= top->phead->run);
DEAD_LIFE:
	pthread_mutex_unlock(&top->phead->lock);
	pthread_cleanup_pop(0);

	if (dead_life) {
		int i;
		unsigned int st = powersave / 12;
		
		while (p2p) {
			tp2p = p2p->next;
			free(p2p);
			p2p = tp2p;
		}
		__debug("DEAD sleep\n");
		_test_exit();
		for (i=0; i<24; i++) {
			sleep(st);
			_test_exit();
		}
		return;
	}

	__ndebug(__GM__);
	if (becancel) {
		while (p2p) {
			tp2p = p2p->next;
			free(p2p);
			p2p = tp2p;
		}
		_test_exit();
		assert(0);
		pthread_exit(0);
	}

	phead = top->phead;
#ifdef __ZHZ_DEBUG__
	memcpy(&mask, &top->addr, sizeof(unsigned int));
	mp = (unsigned char *)&mask;
#endif
#ifdef __SEND_TRUE__ 
#endif
	probe_clean(top);
	__ndebug(__GM__);
	pthread_mutex_lock(&phead->lock);
	if (0 == phead->num) {
		assert(0 == phead->run);
		assert(0==phead->anum && 0==phead->arun);
		assert(list_empty(&phead->probes));
		sem_post(&phead->sem);
	}
	pthread_mutex_unlock(&phead->lock);
	__ndebug(__GM__);
	phead = 0;
	__debug("thread %d (%d.%d.%d.%d) exit!\n" _ getpid() _ mp[0] _ mp[1] _ mp[2] _ mp[3]);
	pthread_exit(0);
}

static probe_init(int *fdi, int *fds, struct probe *top)
{
	size_t size;
	int v, i;
	struct icmp_filter id;
	
	__ndebug("%d\n" _ top->state);
	assert(READY == (top->state));
	top->state = RUN;
	top->pid = getpid();
	top->thd = pthread_self();
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, 0);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
	top->last = 0;
	top->totals = top->silent = top->eups = top->ups = 0;
	top->icmp.hdr.type = ICMP_ECHO;
	top->icmp.hdr.code = 0;
	top->icmp.hdr.un.echo.id = getpid();
	top->icmp.hdr.un.echo.sequence = 0;
	top->icmp.mask = top->mask;
	top->icmp.addr = top->addr;
	top->icmp.sa = top->sa;
	size = sizeof(struct iecho) << 10;
	
	if (-1 == (*fdi=socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)))
		__edebug("socket()" __GM__);
	setsockopt(*fdi, SOL_SOCKET, SO_RCVBUF, &size, sizeof size);
	id.data = top->icmp.hdr.un.echo.id;
	//############## <PLEASE use the CHANGED kernel!> #################
#if 0
	if (0 != setsockopt(*fdi, SOL_RAW, ZHZ_ICMP_FILTER, &id.data, sizeof id.data))
		__debug("setsockopt()" __GM__);
	else 
		__debug("ZHZ_ICMP_FILTER\n");
#endif
	//############################# <END> #############################
	v = fcntl(*fdi, F_GETFL, 0); 
	if (-1 == fcntl(*fdi, F_SETFL, v|O_NONBLOCK))
		__edebug("fcntl()" __GM__);
	
	if (-1 == (*fds=socket(AF_INET, SOCK_DGRAM, 0))) 
		__edebug("socket()" __GM__);
	
	for (i=0; i<ALL_BIND; i++) 
		top->sr[i] = 0;

	top->nextid = 0;
}

static void test_exit(struct probe *top)
{
	if (DEAD == top->state) {
		_test_exit();
	}
}

void *do_probe(void *pppp)
{
	struct probe *top = pppp;
	unsigned int silent = 0;
	unsigned char *p = (unsigned char *)(&top->sa);
	time_t beept = 0, beep = 0;
	unsigned int icnt = 0, firstt;
	int i;
#ifdef __ZHZ_DEBUG__
	unsigned int nn = 0;
	unsigned int aaddr;
	unsigned char *aap = (unsigned char *)&aaddr;
	
	memcpy(&aaddr, &top->sa, sizeof(unsigned int));
#endif
	
	probe_init(&top->fdi, &top->fds, top);
	time(&beept);
	__ndebug("do_probe\n");

	for (firstt=1;;firstt=0) {
		test_exit(top);
		do_ping(top->fdi, top);
		test_exit(top);

		del_down(top, 0);

#ifdef __ZHZ_DEBUG_1__
		__debug("eups == %d, ups == %d -- (%d.%d.%d.%d | %d) <%d> \n" _
		       	top->eups _ top->ups _ aap[0] _ aap[1] _ aap[2] _ 
			aap[3] _ top->anum _ nn++);
#endif

		if (0 == top->ups) {
			top->silent = (get_silent()+1)==(top->silent+1)?0:(top->silent+1);
			if (get_silent()==top->silent || silent) {
				silent++;
				if (UINT_MAX-20 < silent)
					silent /= 2;
			}
		} else {
			top->totals = silent = top->silent = 0;
			test_exit(top);
			if (DEAD==top->state || POWERSAVE == top->state)
				top->state = RUN;

			time(&beep);
			if (get_gmt()<=(beep-beept) || firstt) {
				for (i=0; i<ALL_BIND-2; i++) {
					icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
					top->sr[i] = zhz_getreq(_bind[i].bind, 
						_bind[i].len*ZHZ_GETITM, 
						PT_GETREQ, 0);
					assert(top->sr[i]);
					assert(top->sr[i]->req);
					reuse_getreq(top->sr[i], 
						(unsigned char *)&icnt);
					__ndebug("++ GOING %d SNMP ++\n" _ i);
					do_snmp(top, DO_GET_REQ, i, 0);
#ifdef __ZHZ_DEBUG_1__
					__debug("++ DOWN %d SNMP ++\n" _ i);
#endif
					assert(top->sr[i]);
					assert(top->sr[i]->req);
					free(top->sr[i]->req);
					free(top->sr[i]);
					top->sr[i] = 0;
				}
				icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
				top->sr[IF_BIND] = zhz_getreq(
					_bind[IF_BIND].bind, 
					_bind[IF_BIND].len*ZHZ_IF_ITM, 
					PT_GETREQ, 1);
				assert(top->sr[IF_BIND]);
				assert(top->sr[IF_BIND]->req);
				reuse_getreq(top->sr[IF_BIND], 
					(unsigned char *)&icnt);
				__ndebug("++ GOING if SNMP ++\n");
				do_snmp(top, DO_IF_REQ, 1, 0);
#ifdef __ZHZ_DEBUG_1__
				__debug(" ++ DOWN if SNMP ++\n");
#endif
				assert(top->sr[IF_BIND]);
				assert(top->sr[IF_BIND]->req);
				free(top->sr[IF_BIND]->req);
				free(top->sr[IF_BIND]);
				top->sr[IF_BIND] = 0;
				test_exit(top);
				beept = beep;
			} else
				__debug("%u > %u \n" _ get_gmt() _ (beep-beept));
#if 1
			{
			int yy_;
			for (yy_=0; yy_<12; yy_++) {
#ifdef __ZHZ_DEBUG_1__
				__ndebug("++ GOING ipad SNMP (%d)++\n" _ yy_);
#endif
				icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
				top->nextid = icnt;
				if (do_snmp(top, DO_GETNEXT_REQ, 0, yy_))
					break;
#ifdef __ZHZ_DEBUG_1__
				__debug("++ DOWN ipad SNMP (%d)++\n" _ yy_);
#endif
			}
			}

#if 0
			{
				int yy_;
				top->sr[FLOW_BIND] = zhz_getreq(
					_bind[FLOW_BIND].bind, 
					_bind[FLOW_BIND].len*ZHZ_IF_ITM, 
					PT_GETREQ, 1);
				icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
				reuse_getreq(top->sr[FLOW_BIND], 
					(unsigned char *)&icnt);
				__ndebug("++ GOING flow SNMP++ 0\n");
				do_snmp(top, DO_FLOW_REQ, 1, 0);
				__ndebug("++ GOING flow SNMP out++ 0\n");
				for (yy_=1; yy_<28; yy_++) {
					__ndebug("++ GOING flow SNMP++ %d\n" _ yy_);
					icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
					reuse_getreq(top->sr[FLOW_BIND], 
						(unsigned char *)&icnt);
					if (do_snmp(top, DO_FLOW_REQ, 0, yy_))
						break;
					__ndebug("++ GOING flow SNMP out++ %d\n" _ yy_);
				}
				assert(top->sr[FLOW_BIND]);
				assert(top->sr[FLOW_BIND]->req);
				free(top->sr[FLOW_BIND]->req);
				free(top->sr[FLOW_BIND]);
				top->sr[FLOW_BIND] = 0;
			}
#endif

			{
			int yy_;
			top->sr[IF_BIND] = zhz_getreq(_bind[IF_BIND].bind, 
				_bind[IF_BIND].len*ZHZ_IF_ITM, PT_GETREQ, 1);
			assert(top->sr[IF_BIND]);
			assert(top->sr[IF_BIND]->req);
			for (yy_=1; yy_<12; yy_++) {
				__ndebug(" ++ GOING if SNMP 2 (%d)++\n" _ yy_);
				icnt = (UINT_MAX-20<icnt) ? 0 : icnt+1;
				reuse_getreq(top->sr[IF_BIND], (unsigned char *)&icnt);
				if (do_snmp(top, DO_IF_REQ, 0, yy_))
					break;
#ifdef __ZHZ_DEBUG_1__
				__debug(" ++ DOWN if SNMP 2 (%d)++\n" _ yy_);
#endif
			}
			assert(top->sr[IF_BIND]);
			assert(top->sr[IF_BIND]->req);
			free(top->sr[IF_BIND]->req);
			free(top->sr[IF_BIND]);
			top->sr[IF_BIND] = 0;
			}
			__ndebug("++ DOWN if SNMP2 ++\n");
#endif
		}
		
		top->icmp.hdr.un.echo.sequence++;
		top->last++;
		me2sleep(silent + 1, top);
	}

	return 0;
}

//
// end of probe.c
//

