
/*
 * probe.h
 */

#ifndef _ZHZ_PROBE_H_
#define _ZHZ_PROBE_H_

struct iecho {
	struct icmphdr hdr;
	unsigned int mask;
	unsigned int addr;
	unsigned int sa;
};
	
struct probe {
	unsigned int addr;
	unsigned int sa;
	unsigned int mask;
	int state;
	int start;
	pid_t pid;	  
	pthread_t thd;
	int fdi;
	int fds;
	unsigned int distant;
	unsigned int num;
	unsigned int anum;
	unsigned int eups;
	unsigned int ups;
	unsigned int size;
	unsigned int last; /* probe id */
	unsigned int silent; /* five silent alarm */
	unsigned int totals;
	struct iecho icmp;
	struct probe_head *phead;
	struct snmp_request *sr[ALL_BIND];
	struct list_head *list;
	struct list_head probes;
	struct thr_send *ts;
	unsigned int nextid;
};

struct probe_head {
	int num;
	int run;
	int anum;
	int arun;
	struct list_head probes;
	struct list_head newprobes;
	sem_t sem; // dynamic notify
	pthread_mutex_t lock;
	struct list_head sends;
	struct list_head *ts;
};

struct ifinfo {
	unsigned char ipaddr[4];
	unsigned int ipifindex;
	unsigned char ipmask[4];
	unsigned int ipbcast;
	unsigned int ipmaxsize;
	struct list_head ifinfo;
};

struct up {
	struct list_head list;
	unsigned int addr;
	unsigned int last;
	short eir;
	short enr; // expect ipad next response
	struct snmp_request *nreq;
	unsigned int oid;
	unsigned int ifnum;
	unsigned int isgw;
	struct list_head ifinfo;
	short efr;
	short tf;
	unsigned int *flowin;
	unsigned int *flowout;
};

struct p2probe {
	struct probe *top;
	struct p2probe *next;
};

enum {
	DEF_REST=30, /* 30 seconds */
	IDLETIMES=432000, /* 5 days */
	POWERSAVE=0,
	RUN=1,
	READY=2,
	DEAD=3
};

unsigned int del_down(struct probe *top, int force);
void *do_probe(void *);

#define START_ROLL \
	for (i=0; i<top->size; i++) {\
		for (l=top->list[i].next; l!=&top->list[i]; l=l->next) {\
\

#define END_ROLL \
		}\
	}\
\

				
#endif /* _ZHZ_PROBE_H_ */

/*
 * end of probe.h
 */

