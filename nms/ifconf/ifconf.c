
//
// ifconf.c
//

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ip_icmp.h>

#include "debug.h"
#include "list.h"
#include "ifconf.h"
#include "probe.h"

struct ifi_head {
	int num;
	struct list_head list;
};

struct ifi_head ihead;

static char *get_all(int fd, struct ifconf *ifc) 
{
	char *buf = 0;
	int len, l = 0;
	
	len = 10 * sizeof(struct ifreq);
	for (;;) {
		if (0 == (buf=malloc(len)))
			__edebug("malloc()" __GM__);
		ifc->ifc_len = len;
		ifc->ifc_buf = buf;
		
		if (0 > ioctl(fd, SIOCGIFCONF, ifc))
			__edebug("ioctl()" __GM__);
		if (ifc->ifc_len == l)
			break;
		l = ifc->ifc_len;
		len += 10 * sizeof(struct ifreq);
		free(buf);
	}

	return buf;
}

static void new_ifi(int fd, struct ifreq *ifr)
{
	struct ifi *info = 0;
	struct ifreq cp;

	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
#if 1
	info->distant = 1;
#endif
	
	cp = *ifr;
	if (0 > ioctl(fd, SIOCGIFNETMASK, &cp))
		__edebug("ioctl()" __GM__);
	memcpy(&info->name, &ifr->ifr_name, IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	memcpy(&info->addr, &((struct sockaddr_in *)(&ifr->ifr_addr))->sin_addr.s_addr, sizeof(unsigned int));
	memcpy(&info->mask, &((struct sockaddr_in *)&cp.ifr_addr)->sin_addr.s_addr, sizeof(unsigned int));
	
	list_add(&info->list, &ihead.list);
	ihead.num++;
}

#if 1
static void just_for_test()
{
	struct ifi *info = 0;
	unsigned char a[4], b[4];

#if 0
	b[0] = 255; b[1] = 255; b[2] = 255; b[3] = 0;

	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 6;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 202; a[1] = 106; a[2] = 192; a[3] = 0;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;

	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 2;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 211; a[1] = 101; a[2] = 151; a[3] = 0;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;
#endif

#if 1	
	b[0] = 255; b[1] = 255; b[2] = 255; b[3] = 224;
#endif
	
#if 1	
	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 10;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 61; a[1] = 135; a[2] = 224; a[3] = 0;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;
#endif 

#if 1	
	b[0] = 255; b[1] = 255; b[2] = 255; b[3] = 192;
#endif
	
#if 1	
	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 10;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 61; a[1] = 135; a[2] = 45; a[3] = 145;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;
#endif 


#if 1	
	b[0] = 255; b[1] = 255; b[2] = 255; b[3] = 0;
#endif
#if 1	
	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 10;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 61; a[1] = 135; a[2] = 129; a[3] = 0;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;
#endif 


#if 0
	if (0 == (info=malloc(sizeof(struct ifi))))
		__edebug("malloc()" __GM__);
	INIT_LIST_HEAD(&info->list);
	info->distant = 16;
	memcpy(&info->name, "eth0", IFNAMSIZ);
	info->name[IFNAMSIZ-1] = '\0';
	a[0] = 64; a[1] = 15; a[2] = 0; a[3] = 0;
	memcpy(&info->addr, &a, sizeof(unsigned int));
	memcpy(&info->mask, &b, sizeof(unsigned int));
	list_add(&info->list, &ihead.list);
	ihead.num++;
#endif
}
#endif

static void init_ifi() 
{
	struct ifconf ifc;
	struct ifreq *ifr, cp;
	char *buf = 0, *p = 0;
	char name[IFNAMSIZ];
	int fd;
	
	ihead.num = 0;
	INIT_LIST_HEAD(&ihead.list);
	if (-1 == (fd=socket(AF_INET, SOCK_DGRAM, 0)))
		__edebug("socket()" __GM__);

	p = buf = get_all(fd, &ifc);

#if 0
	while (p < buf+ifc.ifc_len) {
		ifr = (struct ifreq *)p;
		p += sizeof(ifr->ifr_name) + sizeof(struct sockaddr);
		__ndebug("ifname == %s\n" _ ifr->ifr_name);
		if (strchr(ifr->ifr_name, ':') || 
			0 == strncmp(loopdev, ifr->ifr_name, 3))
			continue;
		cp = *ifr;
		if (0 > ioctl(fd, SIOCGIFFLAGS, &cp))
			__edebug("ioctl()" __GM__);
		if (0 == cp.ifr_flags&IFF_UP)
			continue;
		new_ifi(fd, ifr);
	}
#endif

#if 1
	just_for_test();
#endif

	free(buf);
	close(fd);
}
	
#ifdef __ZHZ_TEST_IFCONF__
int main()
{
	struct list_head *l = 0;
	struct ifi *info = 0;
	
	init_ifi();

	for (l=ihead.list.next; l!=&ihead.list; l=l->next) {
		info = list_entry(l, struct ifi, list);
		printf("%s\tADDR:%d.%d.%d.%d\tMASK:%d.%d.%d.%d\n", 
			info->name, 
			((unsigned char *)(&info->addr))[0],
			((unsigned char *)(&info->addr))[1],
			((unsigned char *)(&info->addr))[2],
			((unsigned char *)(&info->addr))[3],
			((unsigned char *)(&info->mask))[0],
			((unsigned char *)(&info->mask))[1],
			((unsigned char *)(&info->mask))[2],
			((unsigned char *)(&info->mask))[3]);
	}
	return 0;	
}
#else
void init_probe(struct probe_head *phead)
{
	struct list_head *l = 0, *ll = 0;
	struct probe *top = 0, *ttop = 0;
	struct ifi *i = 0;
	
	init_ifi();
	l = ihead.list.next;
	
	while (l != &ihead.list) {
		i = list_entry(l, struct ifi, list);
		__ndebug("%s\tADDR:%d.%d.%d.%d\tMASK:%d.%d.%d.%d\n" _ 
			i->name _ 
			((unsigned char *)(&i->addr))[0] _ 
			((unsigned char *)(&i->addr))[1] _ 
			((unsigned char *)(&i->addr))[2] _ 
			((unsigned char *)(&i->addr))[3] _ 
			((unsigned char *)(&i->mask))[0] _ 
			((unsigned char *)(&i->mask))[1] _ 
			((unsigned char *)(&i->mask))[2] _
			((unsigned char *)(&i->mask))[3]);
		l = l->next;
		list_del_init(&i->list);
		if (0 == (top=malloc(sizeof(struct probe))))
			__edebug("malloc()" __GM__);
		memset(top, 0, sizeof(struct probe));
		top->addr = (i->addr&i->mask);
		top->mask = i->mask;
#if 1
		top->distant = i->distant;
#endif
#if 0
		top->distant = 1;
#endif
		top->state = READY;
		
		__ndebug("init_probe into mutex lock\n");
		pthread_mutex_lock(&phead->lock);
		for (ll=phead->probes.next; ll!=&phead->probes; ll=ll->next) {
			ttop = list_entry(ll, struct probe, probes);
			if (0 == memcmp(&top->addr, &ttop->addr, 
				sizeof(unsigned int))) {
				pthread_mutex_unlock(&phead->lock);
				free(top);
				goto NEXT;
			}
		}
		
		__ndebug("init_probe into mutex lock 1\n");
		for (ll=phead->newprobes.next; 
			ll!=&phead->newprobes; ll=ll->next) {
			ttop = list_entry(ll, struct probe, probes);
			if (0 == memcmp(&top->addr, &ttop->addr, 
				sizeof(unsigned int))) {
				pthread_mutex_unlock(&phead->lock);
				__ndebug("init_probe into mutex lock 5\n");
				free(top);
				goto NEXT;
			}
			__ndebug("init_probe into mutex lock 6\n");
		}
		__ndebug("init_probe into mutex lock 2\n");
		list_add(&top->probes, &phead->newprobes);
		sem_post(&phead->sem);
		pthread_mutex_unlock(&phead->lock);
		__ndebug("init_probe into mutex lock 3\n");
NEXT:
		free(i);
		i = 0;
	}
	__ndebug("out of init_probe()\n");
}
#endif

//
// end of ifconf.c
//

