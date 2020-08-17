
//
// trap.c
//

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/icmp.h>

#ifdef __SEND_TRUE__
#endif

#include "debug.h"
#include "list.h"
#include "queue.h"
#include "probe.h"
#include "snmp.h"
#include "send.h"
#include "trap.h"

enum {
	PORT_TRAP = 162
};

struct thr_send *ths = 0;

int get_trap(unsigned char *buf, unsigned int tlen, unsigned char *start)
{
	unsigned char *p = buf;
	unsigned int i = 0, j = 0, l = 0, tl = 0;
	unsigned int id;
	unsigned char oid[257];
	unsigned char ip[16];
	unsigned char gt[30];
	int st = 0;
	unsigned char ts[11];

	memset(oid, 0, 257);
	memset(ip, 0, 16);
	memset(gt, 0, 30);
	memset(ts, 0, 11);
	
	if (T_OID != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &l))
		return 2;
	if (6 > l) {
		__debug("oid len error" __GM__);
		return 2;
	}
	if (0x80 < p[i+5]) {
		if (7 > l) {
			__debug("oid len error" __GM__);
			return 2;
		}
		id = 0x80 * (p[i+5]-0x80) + p[i+6];
		assert(7 == l);
	} else {
		id = p[i+5];
		assert(6 == l);
	}
	snprintf(oid, 256, "1.3.6.1.4.1.%u", id);
	i += l;

	if (T_IPADDR != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &l))
		return 2;
	if (4 != l)
		return 2;
	snprintf(ip, 16, "%u.%u.%u.%u", p[i], p[i+1], p[i+2], p[i+3]);
	i += l;
	
	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &l))
		return 2;
	for (j=0, tl=0; j<l; j++)
		tl = (tl<<8) + p[i+j];
	if (0 == tl)
		strcpy(gt, "device cold start");
	else if (1 == tl)
		strcpy(gt, "device warm start");
	else if (2 == tl)
		strcpy(gt, "device linkdown");
	else if (3 == tl)
		strcpy(gt, "device linkup");
	else if (4 == tl)
		strcpy(gt, "device authentication failue");
	else if (5 == tl)
		strcpy(gt, "device egp neighbour loss");
	else if (6 == tl)
		strcpy(gt, "enterprise specific trap infomation");
	else 	
		assert(0);
	i += l;

	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &l))
		return 2;
	for (j=0, st=0; j<l; j++)
		st = (st<<8) + p[i+j];
	i += l;

	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &l))
		return 2;
	for (j=0, tl=0; j<l; j++)
		tl = (tl<<8) + p[i+j];
	tl /= 100;
	snprintf(ts, 10, "%u", tl);
	i += l;

#ifdef __SEND_TRUE__
#if 0
	assert(ths);
#endif
#endif
	
	return tlen + buf - start;
}

void *do_trap(void *noop)
{
	int fd, r;
	struct sockaddr_in sa, addr;
	int l = sizeof addr;
	unsigned char buf[65536];
	
	ths = noop;

	sa.sin_family = PF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons(PORT_TRAP);

	if (-1 == (fd=socket(AF_INET, SOCK_DGRAM, 0))) 
		__edebug("socket()" __GM__);
	if (-1 == bind(fd, (struct sockaddr *)&sa, sizeof sa))
		__edebug("bind()" __GM__);
	
	for (;;) {
		memset(&addr, 0, sizeof addr);
		memset(buf, 0, sizeof buf);
		addr.sin_family = PF_INET;
		l = sizeof addr;
		
		__debug("do_trap is waiting SNMP-TRAP\n");
		r = recvfrom(fd, buf, sizeof buf, 0, (struct sockaddr *)&addr, 
			&l);
		if (-1 == r) {
			__debug("recvfrom()" __GM__);
			continue;
		}
		if (l != sizeof addr) {
			__debug("wrong sockaddr length!\n");
			continue;
		}
		__debug("do_trap recvive a SNMP-TRAP\n");
		
		if (r != prase_resp(buf, 0, 0, DO_TRAP, 0))
			__debug("do_trap error in prasing SNMP-TRAP \n" __GM__);
		else
			__debug("do_trap recvive a correct SNMP-TRAP\n");
	}

	return 0;	
}

//
// end of trap.c
//

