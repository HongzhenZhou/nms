
//
// echo.c
//

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <linux/types.h>
#include <linux/icmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __SEND_TRUE__ 
#endif

#include "debug.h"
#include "list.h"
#include "queue.h"
#include "probe.h"
#include "send.h"
#include "echo.h"

static unsigned short in_cksum(struct iecho *p, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short *w = (unsigned short *)p;
	unsigned short s = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *)(&s) = *(unsigned char *)w;
		sum += s;
	}

	sum = (sum>>16) + (sum&0xffff);
	sum += (sum >> 16);
	s = ~sum;
	return s;
}

int send_echo(int fd, struct probe *top, unsigned int *a, struct list_head *l)
{
	int i, t, len = sizeof(struct iecho);
	struct sockaddr_in addr;
	struct up *pup;
	
	top->icmp.hdr.checksum = 0;
	top->icmp.hdr.checksum = in_cksum(&top->icmp, len);

	addr.sin_family = AF_INET;

	if (0 != a) {
		unsigned char *p = (unsigned char *)&addr.sin_addr.s_addr;
		assert(0 == l);
		memcpy(&addr.sin_addr.s_addr, a, sizeof(unsigned int));
		__ndebug("send echo to %d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
	} else {
		assert(l);
		pup = list_entry(l, struct up, list);
		addr.sin_addr.s_addr = pup->addr;
	}
	
	if (-1 == sendto(fd, &top->icmp, len, 0, (struct sockaddr *)&addr, sizeof addr)) {
#ifdef __ZHZ_DEBUG__
		char buf[1024];
		strerror_r(errno, buf, sizeof buf);
		perror(buf);
		__ndebug("sendto()" __GM__);
#endif
		return -1;
	}
	__ndebug("- %d " _ i);
	__ndebug("\n");

	return 0;
}

unsigned int get_key(unsigned int *a, unsigned int *b, unsigned int size)
{
	unsigned int key = ( 
		(((unsigned char *)&a)[0]-((unsigned char *)&b)[0])<<24 + 
		(((unsigned char *)&a)[1]-((unsigned char *)&b)[1])<<16 + 
		(((unsigned char *)&a)[2]-((unsigned char *)&b)[2])<<8 + 
		(((unsigned char *)&a)[3]-((unsigned char *)&b)[3]) 
		) & size;
	
	return key;
}

static int newup(unsigned int addr, struct probe *top)
{
	struct up *pup;
	unsigned int key, i;
	unsigned int a = addr&top->mask;
	unsigned int b;
	unsigned int c;
	unsigned char *p1 = (unsigned char *)(&c);
	unsigned char *p2 = (unsigned char *)(&top->sa);
	unsigned char *p3 = (unsigned char *)(&b);
	unsigned char *p4 = (unsigned char *)(&addr);
	int vc = 0;

	__ndebug("into newup" __GM__);
	for (i=0; i<4; i++) 
		p1[i] = p2[3-i];
	for (i=0; i<4; i++) 
		p3[i] = p4[3-i];
	vc = b-c;
	
	__ndebug("newup" __GM__);
	if (!memcmp(&a, &top->addr, sizeof a) && 
		vc>0 && vc<=top->num){
#ifdef __SEND_TRUE__ 
#endif
		
		__ndebug("It's in this net's mask\n");
		pup = malloc(sizeof(struct up));
		if (0 == pup) 
			return -2;
		memset(pup, 0, sizeof(struct up));
		assert(0 == pup->enr);
		assert(0 == pup->nreq);
		INIT_LIST_HEAD(&pup->list);
		INIT_LIST_HEAD(&pup->ifinfo);
		pup->last = top->last;
		pup->addr = addr;
		pup->eir = 0;
		pup->efr = 0;
		pup->tf = 0;
		pup->flowin = 0;
		pup->flowout = 0;
		key = get_key(&addr, &top->sa, top->size);
		list_add(&pup->list, &top->list[key]);
		top->ups++;
		top->eups++;
		assert(0 == pup->enr);
		assert(0 == pup->nreq);

#ifdef __SEND_TRUE__ 
#endif
out:
		__ndebug("newup -- %d.%d.%d.%d -> last == %d\n" _ 
			p[0] _ p[1] _ p[2] _ p[3] _ pup->last);
		__ndebug("newup : top->eups == %d, top->ups == %d\n" _ 
			top->eups _ top->ups);
		return 0;
	}

	__ndebug("newup NOT found\n");
	return 1;
}

struct up *ckaddr(struct sockaddr_in *addr, struct probe *top)
{
	struct list_head *l;
	unsigned int key;
	struct up *pup;

	key = get_key(&addr->sin_addr.s_addr, &top->sa, top->size);
	
	for (l=top->list[key].next; l!=&top->list[key]; l=l->next) {
		pup = list_entry(l, struct up, list);
		if (!memcmp(&pup->addr, &addr->sin_addr.s_addr, sizeof(unsigned int))) 
			return pup;
	}

	return 0;
}

int recv_echo(int fd, struct probe *top)
{
	struct iecho *answer;
	int n = 0, r = 0;
	char buf[256];
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	struct up *pup = 0;
	unsigned char *p = (unsigned char *)(&addr.sin_addr.s_addr);
	
loop:
	n = recvfrom(fd, buf, sizeof buf, 0, (struct sockaddr *)&addr, &len);

	if (0 > n)
	       return -1;
	if (len != sizeof(struct sockaddr_in)) {
		__debug("wrong sockaddr length!\n");
		goto loop;
	}
	if ((n-((struct ip *)buf)->ip_hl<<2) < (sizeof(struct iecho))) {
		goto loop;
	}
	__ndebug("recv echo from %d.%d.%d.%d\n" _ 
			p[0] _ p[1] _ p[2] _ p[3]);

	answer = (struct iecho *)(buf + (((struct ip *)buf)->ip_hl<<2));
	if (ICMP_ECHOREPLY!=answer->hdr.type || 
		answer->hdr.un.echo.id!=top->icmp.hdr.un.echo.id || 
		answer->hdr.un.echo.sequence!=top->icmp.hdr.un.echo.sequence || 
		memcmp(&answer->addr, &top->addr, sizeof(unsigned int)) ||
		memcmp(&answer->sa, &top->sa, sizeof(unsigned int)) ||
		memcmp(&answer->mask, &top->mask, sizeof(unsigned int))) {
		goto loop;
	}
	
	pup = ckaddr(&addr, top);
	if (pup) {
		__ndebug("recv echo from %d.%d.%d.%d\n" _  
			p[0] _ p[1] _ p[2] _ p[3]);
		__ndebug("%d \n" _ top->last);
		if (pup->last < top->last) {
			__ndebug("old update -- %d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
			pup->last = top->last;
			top->ups++;
			__ndebug("top->ups == %d\n" _ top->ups);
		}
		goto loop;
	}
		
	__ndebug("old NOT found, into newup\n");
	newup(addr.sin_addr.s_addr, top);
	__ndebug("out of newup\n");
	goto loop;

	return 0;
}

//
// end of echo.c
//

