
//
// snmp.c
//

#include <assert.h>
#define __USE_UNIX98
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/icmp.h>
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
#if 0
#include "trap.h"
#endif

enum {
	SNMP_VER1=0,
	SNMP_VER2=1,
	LEN_ID=0x80,
	PORT_SNMP=161
};

#define	LOOPBACK(a)	((((long int)(a))&htonl(0xff000000))==htonl(0x7f000000))
#define	PRINET1(a)	((((long int)(a))&htonl(0xff000000))==htonl(0x0a000000))
#define	PRINET2(a)	((((long int)(a))&htonl(0xffff0000))==htonl(0xc0a80000))
#define MULTICAST(x)	(((x) & htonl(0xf0000000)) == htonl(0xe0000000))
#define BADCLASS(x)	(((x) & htonl(0xf0000000)) == htonl(0xf0000000))
#define ZERONET(x)	(((x) & htonl(0xff000000)) == htonl(0x00000000))

///////////////////////////////////////////////////////////////////////////////

unsigned char sys_bind[ZHZ_SYS_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x4,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x5,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x6,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x1,0x7,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x2,0x1,0x0,T_NULL,0x0
};

unsigned char ip_bind[ZHZ_IP_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x4,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x5,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x6,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x7,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x8,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x9,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xa,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xb,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xc,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xd,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xe,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0xf,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x10,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x11,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x12,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x4,0x13,0x0,T_NULL,0x0
};

unsigned char icmp_bind[ZHZ_ICMP_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x4,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x5,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x6,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x7,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x8,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x9,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xa,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xb,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xc,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xd,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xe,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0xf,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x10,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x11,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x12,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x13,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x14,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x15,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x16,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x17,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x18,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x19,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x5,0x1a,0x0,T_NULL,0x0
};

unsigned char tcp_bind[ZHZ_TCP_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x4,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x5,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x6,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x7,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x8,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0x9,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0xa,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0xb,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0xc,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0xe,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x6,0xf,0x0,T_NULL,0x0
};

unsigned char udp_bind[ZHZ_UDP_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x7,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x7,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x7,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0x7,0x4,0x0,T_NULL,0x0
};

unsigned char snmp_bind[ZHZ_SNMP_NUM*ZHZ_GETITM] = {
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x2,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x3,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x4,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x5,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x6,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x8,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x9,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xa,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xb,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xc,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xd,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xe,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0xf,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x10,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x11,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x12,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x13,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x14,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x15,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x16,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x18,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x19,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1a,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1b,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1c,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1d,0x0,T_NULL,0x0,
	T_SEQ,0xc,T_OID,0x8,0x2b,0x6,0x1,0x2,0x1,0xb,0x1e,0x0,T_NULL,0x0
};

unsigned char ipad_next[ZHZ_IPAD_NEXT_NUM*ZHZ_GETNEXT_ITM] = {
	T_SEQ,0xd,T_OID,0x9,0x2b,0x6,0x1,0x2,0x1,0x4,0x14,0x1,0x1,T_NULL,0x0,
	T_SEQ,0xd,T_OID,0x9,0x2b,0x6,0x1,0x2,0x1,0x4,0x14,0x1,0x2,T_NULL,0x0,
	T_SEQ,0xd,T_OID,0x9,0x2b,0x6,0x1,0x2,0x1,0x4,0x14,0x1,0x3,T_NULL,0x0,
	T_SEQ,0xd,T_OID,0x9,0x2b,0x6,0x1,0x2,0x1,0x4,0x14,0x1,0x4,T_NULL,0x0,
	T_SEQ,0xd,T_OID,0x9,0x2b,0x6,0x1,0x2,0x1,0x4,0x14,0x1,0x5,T_NULL,0x0
};

unsigned char flow_bind[ZHZ_FLOW_NUM*ZHZ_IF_ITM] = {
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xa,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x10,0x1,T_NULL,0x0
};

unsigned char if_bind[ZHZ_IF_NUM*ZHZ_IF_ITM] = {
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x1,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x2,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x3,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x4,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x5,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x6,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x7,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x8,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x9,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xa,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xb,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xc,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xd,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xe,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0xf,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x10,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x11,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x12,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x13,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x14,0x1,T_NULL,0x0,
	T_SEQ,0xe,T_OID,0xa,0x2b,0x6,0x1,0x2,0x1,
	0x2,0x2,0x1,0x15,0x1,T_NULL,0x0
};

struct snmp_bind _bind[ALL_BIND] = {
	{sys_bind, ZHZ_SYS_NUM},
	{ip_bind, ZHZ_IP_NUM},
	{icmp_bind, ZHZ_ICMP_NUM},
	{tcp_bind, ZHZ_TCP_NUM},
	{udp_bind, ZHZ_UDP_NUM},
	{snmp_bind, ZHZ_SNMP_NUM},
	{if_bind, ZHZ_IF_NUM},
	{flow_bind, ZHZ_FLOW_NUM}
};


struct snmp_bind _next[ALL_NEXT] = {
	{ipad_next, ZHZ_IPAD_NEXT_NUM}
};

///////////////////////////////////////////////////////////////////////////////

int cklen(unsigned char *p, unsigned int *i, unsigned int *ool)
{
	int l, j;
	
	l = p[(*i)++];
	if (LEN_ID < l) {
		if (4<(l-LEN_ID) || 1>(l-LEN_ID)) {
			__debug("%d\n" _ (l-LEN_ID));
			__debug(__GM__);
			return -1;
		}
		for (j=0, *ool=0; j<(l-LEN_ID); j++)
			*ool = (*ool<<8) + p[*i+j];
		*i += (l-LEN_ID);
	} else if (LEN_ID > l) {
		*ool = l;
#if 0
		if (0 == l)
			__debug("0 == l -- vl == %u\n" _ *ool);
#endif
	} else {
		__debug(__GM__);
		return -1;
	}

	return l;
}

unsigned char *add_asn(unsigned char type, int add, unsigned int tl,
	unsigned int vl, unsigned char *val, unsigned int al,
	unsigned char *attach, unsigned int *rl)
{
	unsigned int i = 0, k = 0, l;
	unsigned char *p = 0;
	
	assert(rl);
	assert(tl>=vl && tl>=al && tl==(al+vl));

	l = add ? tl : vl;
	if (LEN_ID <l)
		for (i=1; i<5; i++) {
			if (0 == (l>>(8*i))) 
				break;
			assert(4 != i);
		}
	*rl = tl + i + 2;
	
	if (0 ==(p=malloc(*rl)))
		__edebug("malloc()" __GM__);

	p[k++] = type;

	if (i) {
		int j;
		
		p[k++] = LEN_ID + i;
		for (j=0; j<i; j++)
			p[k++] = ( (l<<((4 -i+j)*8)) >> 24);
	} else
		p[k++] = l;

	if (0 == vl) 
		assert(0 == val);
	else {
		assert(val);
		memcpy(p+k, val, vl);
	}

	if (0 == al)
		assert(0 == attach);
	else {
		assert(attach);
		memcpy(p+k+vl, attach, al);
	}

	return p;
}

struct snmp_request *reuse_getreq(struct snmp_request *p, unsigned char *thus)
{
	int i;

	assert(thus);
	__ndebug("req id len == %d %d %d \n" _ p->req_id[0] _ p->req_id[1] _ p->req_id[2]);
	assert(4 == p->req_id[1]);
	memcpy(p->req_id+2, thus, 4);
		
	__ndebug("%d-%d-%d-%d\n" _ p->req_id[2] _ p->req_id[3] _ p->req_id[4] _ p->req_id[5]);

	return p;
}
		
struct snmp_request *zhz_getreq(unsigned char *zhz_bind, size_t bind_size,
	unsigned char REQ, int vsp)
{
	struct snmp_request *p = 0;
	unsigned char *tp = 0;
	unsigned int tl = 0;
	unsigned char ta[7];
	unsigned int idd = 0, iddl = 0;
	
	if (0 == (p=malloc(sizeof(struct snmp_request))))
		__edebug("malloc()" __GM__);
	memset(p, 0, sizeof(struct snmp_request));
	memset(ta, 0, sizeof ta);
	
	p->req = add_asn(T_SEQ, 1, bind_size, 0, 0, 
		bind_size, zhz_bind, &p->req_len);
	tl = p->req_len;
	tp = p->req;
	p->req = add_asn(T_INT, 0, tl+1, 1, ta, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	tp = p->req;
	p->req = add_asn(T_INT, 0, tl+1, 1, ta, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	tp = p->req;
	__ndebug("%d\n" _ tl);
	p->req = add_asn(T_INT, 0, tl+4, 4, ta, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	tp = p->req;
	idd = 0;
	iddl = tl;
	__ndebug("%d\n" _ tl);
	__ndebug("req id %d-%d-%d-%d\n" _ *(tp+2) _ *(tp+3) _ *(tp+4) _ *(tp+5));
	p->req = add_asn(REQ, 1, tl, 0, 0, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	tp = p->req;
	idd += tl - iddl;
	iddl = tl;
#if 0
	strncpy(ta, "public", sizeof ta);
	p->req = add_asn(T_STR, 0, tl+6, 6, ta, tl, tp, &p->req_len);
#endif
	{
	unsigned int comml;
	unsigned char comm[256];

	memset(comm, 0, 256);
	get_comm(comm, &comml);
	p->req = add_asn(T_STR, 0, tl+comml, comml, comm, tl, tp, &p->req_len);
	}
	free(tp);
	tl = p->req_len;
	tp = p->req;
	idd += tl - iddl;
	iddl = tl;
	ta[0] = SNMP_VER1;
	p->req = add_asn(T_INT, 0, tl+1, 1, ta, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	tp = p->req;
	idd += tl - iddl;
	iddl = tl;
	p->req = add_asn(T_SEQ, 1, tl, 0, 0, tl, tp, &p->req_len);
	free(tp);
	tl = p->req_len;
	idd += tl - iddl;

	p->req_id = p->req + idd;
	if (vsp)
		p->vsp = p->req + tl - bind_size;
	else
		p->vsp = 0;

	return p;
}
	
//////////////////////////////////////////////////////////////////////////////

int send_get(struct up*pup, struct probe *top, struct sockaddr_in *addr, int i)
{
	int k;

	assert(SYS_BIND<=i && IF_BIND>i);
	
	addr->sin_addr.s_addr = pup->addr;
	k = sendto(top->fds, top->sr[i]->req, top->sr[i]->req_len, 0, 
		(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (-1 == k) {
		__debug("sendto()" __GM__);
		return 1;
	}
	__ndebug("sendto success" __GM__);

	return 0;
}

int send_flow(struct up*pup, struct probe *top, struct sockaddr_in *addr, 
	unsigned int i)
{
	int k;

	assert(pup && top);
	assert(0 < i);
	assert(i == pup->efr);
	if (i>512 || i>pup->ifnum) {
		__debug(__GM__);
		return 2;
	}

	for (k=0; k<ZHZ_FLOW_NUM; k++)
		top->sr[FLOW_BIND]->vsp[13+k*ZHZ_IF_ITM] = i;
	
	addr->sin_addr.s_addr = pup->addr;
	k = sendto(top->fds, top->sr[FLOW_BIND]->req, 
		top->sr[FLOW_BIND]->req_len, 0, 
		(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (-1 == k) {
		__debug("sendto()" __GM__);
		return 1;
	}
#if 0
#ifdef __ZHZ_DEBUG__
	{
	unsigned char *p = 0;
	p = (unsigned char *)&addr->sin_addr.s_addr;
	__ndebug("send next to %d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
	__ndebug("send next success" __GM__);
	}
#endif
#endif

	return 0;
}

int send_if(struct up*pup, struct probe *top, struct sockaddr_in *addr, 
	unsigned int i)
{
	int k;

	assert(pup && top);
	assert(0 < i);
	assert(i == pup->eir);
	if (i>512 || i>pup->ifnum) {
		__debug(__GM__);
		return 2;
	}

	for (k=0; k<ZHZ_IF_NUM; k++)
		top->sr[IF_BIND]->vsp[13+k*ZHZ_IF_ITM] = i;
	
	addr->sin_addr.s_addr = pup->addr;
	k = sendto(top->fds, top->sr[IF_BIND]->req, top->sr[IF_BIND]->req_len, 
		0, (struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (-1 == k) {
		__debug("sendto()" __GM__);
		return 1;
	}
#if 0
#ifdef __ZHZ_DEBUG__
	{
	unsigned char *p = 0;
	p = (unsigned char *)&addr->sin_addr.s_addr;
	__ndebug("send next to %d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
	__ndebug("send next success" __GM__);
	}
#endif
#endif

	return 0;
}

int send_next(struct up*pup, struct probe *top, struct sockaddr_in *addr)
{
	int k;
	unsigned char *p = 0;

	assert(pup->enr);
	assert(pup->nreq);
	reuse_getreq(pup->nreq, (unsigned char *)&top->nextid);

	addr->sin_addr.s_addr = pup->addr;
	k = sendto(top->fds, pup->nreq->req, pup->nreq->req_len,
		0,(struct sockaddr *)addr, sizeof(struct sockaddr_in));
	if (-1 == k) {
		__debug("sendto()" __GM__);
		return 1;
	}
	p = (unsigned char *)&addr->sin_addr.s_addr;
	__ndebug("send next to %d.%d.%d.%d\n" _ p[0] _ p[1] _ p[2] _ p[3]);
	__ndebug("send next success" __GM__);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

static unsigned char *_get_val_flow(unsigned char *buf, struct up *pup)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	unsigned int vl = 0;

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl)) {
		__debug(__GM__);
		return 0;
	}
	if (T_OID != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl)) {
		__debug(__GM__);
		return 0;
	}
	for (j=0; j<ZHZ_FLOW_NUM; j++) {
		if (0==memcmp(&p[i], _bind[FLOW_BIND].bind+j*(ZHZ_IF_ITM)+4, 9)
			|| p[i+9]!=pup->efr)
			break;
	}
	if (ZHZ_FLOW_NUM == j) {
		__debug(__GM__);
		return 0;
	}
	i += 10;

	__ndebug("j == %d, type == %d\n" _ j _ p[i]);
	if (T_INT!=p[i] && T_COUNT!=p[i]) {
		__debug("type == %d, j == %d\n" _ p[i] _ j);
		__debug("type error" __GM__);
		return 0;
	}
	i++;
	if (-1 == cklen(p, &i, &vl)) {
		__debug(__GM__);
		return 0;
	}
	
	{
		int jj;
		for (jj=0, l=0; jj<vl; jj++)
			l = (l<<8) + p[i+jj];
	}

	assert(pup->efr <= pup->tf);
	assert(0 < pup->efr);
	
	switch (j) {
	case 0:
		pup->flowin[pup->efr-1] = l;
		pup->flowin[pup->efr-1] = 
			(UINT_MAX-1000)<pup->flowin[pup->efr-1] ? 
			0 : pup->flowin[pup->efr-1];
		break;
	case 1:
		pup->flowout[pup->efr-1] = l;
		pup->flowout[pup->efr-1] = 
			(UINT_MAX-1000)<pup->flowout[pup->efr-1] ? 
			0 : pup->flowout[pup->efr-1];
		break;
	default:__debug("out of range" __GM__);
		return 0;
	}
		
	return p + i + vl;
}

static int get_val_flow(unsigned char *buf, unsigned int tl, struct up *pup, 
	struct probe *top)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	int ret = 2;

	for (i=0; i<_bind[FLOW_BIND].len; i++) {
		p = _get_val_flow(p, pup);
		if (0 == p) {
			__debug("return error" __GM__);
			goto err;
		}
		if (tl < p - buf) {
			__debug("length error" __GM__);
			goto err;
		}
	}
	
	if (tl != p - buf) {
		__debug("length error" __GM__);
		goto err;
	}

#ifdef __SEND_TRUE__
#endif

#ifdef __FLOW_DEBUG__
	__debug("FLOW -- \n");
	__debug("\t%u ++ (%u -- %u)\n" _ pup->efr _ pup->flowin[pup->efr-1] _ 
		pup->flowout[pup->efr-1]);
	__debug("FLOW -- END\n");
#endif
	
	ret = 0;

err:
	__ndebug("out of get_val_flow()\n");
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

static unsigned char *_get_val_if(unsigned char *buf, struct up *pup,
	unsigned char (*sysv)[256], unsigned int *hl)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	unsigned int vl = 0;
	unsigned int rsl = 0, oid = 0;

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl))
		return 0;
	if (T_OID != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl))
		return 0;
	for (j=0; j<ZHZ_IF_NUM; j++) {
		if (0==memcmp(&p[i], _bind[IF_BIND].bind+j*(ZHZ_IF_ITM)+4, 9) ||
			p[i+9]!=pup->eir)
			break;
	}
	if (ZHZ_IF_NUM == j) {
		__debug(__GM__);
		return 0;
	}
	i += 10;

	__ndebug("j == %d, type == %d\n" _ j _ p[i]);
	if (T_STR!=p[i] && (1==j || 5==j)) {
		__debug("type error" __GM__);
		return 0;
	} 
	if (T_INT!=p[i] && T_GAUGE!=p[i] && T_COUNT!=p[i] && T_TIME!=p[i] && 1!=j && 5!=j) {
		__debug("type == %d, j == %d\n" _ p[i] _ j);
		__debug("type error" __GM__);
		return 0;
	}
	i++;
	if (-1 == cklen(p, &i, &vl))
		return 0;
	
	if (1!=j && 5!=j) {
		int jj;
		for (jj=0, l=0; jj<vl; jj++)
			l = (l<<8) + p[i+jj];
	}

	switch (j) {
	case 1:case 5:
		{
		int _vl_;
	       	if (255 < vl) {
			__debug("%d %d %d %d %d %d %d %d %d %d \n" _ 
				*(p-4) _ *(p-3) _ *(p-2) _ *(p-1) _ *(p) _ 
				*(p+1) _ *(p+2) _ *(p+3) _ *(p+4) _ *(p+5)); 
			__debug("str len == %d\n" _ vl);
			__debug("str len > 255" __GM__);
			return 0;
		}
		if (vl)
			memcpy(sysv[j], p+i, vl);
		sysv[j][vl] = '\0';
		_vl_ = strlen(sysv[j]);
		hl[j] = _vl_ + 1;
		__ndebug(" %d | |%s|\n" _ _vl_ _ sysv[j]);
		}
		break;
	case 0:case 2:case 3:case 4:case 6:case 7:case 8:case 9:case 10:
	case 11:case 12:case 13:case 14:case 15:case 16:case 17:case 18:
	case 19:case 20:
			rsl = sprintf(sysv[j], "%u", l);
		if (10 < rsl) {
			__debug(":::::::::: |%s|\n" _ sysv[j]);
			__edebug(":::::::::: 10 < %d (%d)\n" _ rsl _ j);
		}
		assert(10 >= rsl);
		*(hl+j) = rsl + 1;
		break;
	default:__debug("out of range" __GM__);
		return 0;
	}
		
	return p + i + vl;
}

static int get_val_if(unsigned char *buf, unsigned int tl, struct up *pup, 
	struct probe *top)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	unsigned char sysv[ZHZ_IF_NUM][256];
       	unsigned int hl[ZHZ_IF_NUM];
	int ret = 2;

	memset(sysv, 0, ZHZ_IF_NUM*256);
	memset(hl, 0, 4*ZHZ_IF_NUM);
	
	for (i=0; i<_bind[IF_BIND].len; i++) {
		p = _get_val_if(p, pup, sysv, hl);
		if (0 == p) {
			__debug("return error" __GM__);
			goto err;
		}
		if (tl < p - buf) {
			__debug("length error" __GM__);
			goto err;
		}
	}
	
	if (tl != p - buf) {
		__debug("length error" __GM__);
		goto err;
	}

	for (i=0; i<_bind[IF_BIND].len; i++) {
		int sl = 0;
		
		sl=strlen(sysv[i]);
		if (1 > *(hl+i)) {
			__debug("STRAN" __GM__);
			goto err;
		}
#if 1
		if (sl+1 != hl[i]) {
			__edebug("(%d) %d+1 != %d\n" _ i _ sl _ hl[i]);
		}
#endif
		assert(sl+1 == *(hl+i));
		l += sl + 1;
	}

#ifdef __SEND_TRUE__
#endif

#ifdef __IF_DEBUG__
	__debug("IF -- \n");
	for (i=0; i<_bind[IF_BIND].len; i++)
		__debug("\t(%d)==(%s)\n" _ i _ sysv[i]);
	__debug("IF -- END\n");
#endif
	
	ret = 0;

err:
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

static unsigned char *_get_val_sys(unsigned char *buf, struct up *pup,
	unsigned char (*sysv)[256], unsigned int *hl)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	unsigned int vl = 0;
	unsigned int rsl = 0, oid = 0;

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl)) {
		__debug(__GM__);
		return 0;
	}
	if (T_OID != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl)) {
		__debug(__GM__);
		return 0;
	}
	for (j=0; j<ZHZ_SYS_NUM; j++) {
		if (0 == memcmp(&p[i], _bind[SYS_BIND].bind+j*(ZHZ_GETITM)+4, 8))
			break;
	}
	if (ZHZ_SYS_NUM == j) {
		__debug(__GM__);
		return 0;
	}
	i += 8;

	__ndebug("j == %d, type == %d\n" _ j _ p[i]);
	if (T_STR!=p[i] && 1!=j && 2!=j && 6!=j && 7!=j) {
		__debug("type error" __GM__);
		return 0;
	} 
	if (T_OID!=p[i] && 1==j) {
		__debug("type error" __GM__);
		return 0;
	}
	if (T_TIME!=p[i] && 2==j) {
		__debug("type == %d, j == %d\n" _ p[i] _ j);
		__debug("type error" __GM__);
		return 0;
	}
	if (T_INT!=p[i] && (6==j || 7==j)) {
		__debug("type == %d, j == %d\n" _ p[i] _ j);
		__debug("type error" __GM__);
		return 0;
	}
	i++;
	if (-1 == (l=cklen(p, &i, &vl))) {
		__debug(__GM__);
		return 0;
	}
	assert(l || 0==vl);
#if 0
	if (0 == l)
		__debug("0 ==l : It's %d\n" _ j);
#endif
	
	if (2==j || 6==j || 7==j) {
		int jj;
		for (jj=0, l=0; jj<vl; jj++)
			l = (l<<8) + p[i+jj];
	}

	__ndebug("into switch" __GM__);
	switch (j) {
	case 0:case 3:case 4:case 5:
		{
		int _vl_;
	       	if (255 < vl) {
			__debug("%d %d %d %d %d %d %d %d %d %d \n" _ 
				*(p-4) _ *(p-3) _ *(p-2) _ *(p-1) _ *(p) _ 
				*(p+1) _ *(p+2) _ *(p+3) _ *(p+4) _ *(p+5)); 
			__debug("str len == %d\n" _ vl);
			__debug("str len > 255" __GM__);
			return 0;
		}
		if (vl)
			memcpy(sysv[j], p+i, vl);
		sysv[j][vl] = '\0';
		_vl_ = strlen(sysv[j]);
		hl[j] = _vl_ + 1;
		__ndebug(" %d | |%s|\n" _ _vl_ _ sysv[j]);
		}
#if 0
		if (3==j || 5==j)
			__debug("%d | |%s|\n" _ hl[j] _ sysv[j]);
#endif
		break;
	case 1:
		if (6 > vl) {
			__debug("oid len error" __GM__);
			return 0;
		}
		if (0x80 < p[i+5]) {
			if (7 > vl) {
				__debug("oid len error" __GM__);
				return 0;
			}
			oid = 0x80 * (p[i+5]-0x80) + p[i+6];
		} else
			oid = p[i+5];
		pup->oid = oid;
		rsl = sprintf(sysv[j], "1.3.6.1.4.1.%u", oid);
		*(hl+j) = rsl + 1;
		break;
	case 6:case 7:
		rsl = sprintf(sysv[j], "%u", l);
		assert(10 >= rsl);
		*(hl+j) = rsl + 1;
		break;
	case 2:
		{
			unsigned int days = l/8640000;
			unsigned int hours = (l-days*8640000)/360000;
			unsigned int minus = (l-days*8640000-hours*360000)/6000;
			unsigned int seconds = (l-days*8640000-hours*360000-minus*6000)/100;
			
			rsl = sprintf(sysv[j], "%u days, %u hours, %u minutes, %u seconds", days, hours, minus, seconds);
			*(hl+j) = rsl + 1;
		}
		break;
	default:__debug("out of range" __GM__);
		return 0;
	}
		
	return p + i + vl;
}

static int get_val_sys(unsigned char *buf, unsigned int tl, struct up *pup, 
	struct probe *top)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0;
	unsigned char sysv[ZHZ_SYS_NUM][256];
       	unsigned int hl[ZHZ_SYS_NUM];
	int ret = 2;

	memset(sysv, 0, ZHZ_SYS_NUM*256);
	memset(hl, 0, ZHZ_SYS_NUM*4);
	
	for (i=0; i<_bind[SYS_BIND].len; i++) {
		p = _get_val_sys(p, pup, sysv, hl);
		if (0 == p) {
			__debug("return error" __GM__);
			goto err;
		}
		if (tl < p - buf) {
			__debug("length error" __GM__);
			goto err;
		}
	}
	
	if (tl != p - buf) {
		__debug("length error" __GM__);
		goto err;
	}

	for (i=0; i<_bind[SYS_BIND].len; i++) {
		int sl = 0;
		
		sl=strlen(sysv[i]);
		if (1 > hl[i]) {
			__debug("STRAN" __GM__);
			goto err;
		}
		assert(sl+1 == hl[i]);
		l += sl + 1;
	}

#ifdef __SEND_TRUE__
#endif

#ifdef __SYS_DEBUG__
	__debug("SYS -- \n");
	for (i=0; i<_bind[0].len; i++)
		__debug("\t(%d)==(%s)\n" _ i _ sysv[i]);
	__debug("SYS -- END\n");
#endif
	
	pup->ifnum = atoi(sysv[_bind[SYS_BIND].len-1]);
	pup->ifnum = 0==pup->ifnum ? 1 : pup->ifnum;

	if (0 == pup->tf) {
		pup->tf = 0==pup->ifnum ? 1 : pup->ifnum;
		assert(0 == pup->flowin);
		assert(0 == pup->flowout);
		if (0 == (pup->flowin=malloc(pup->tf * sizeof(int)))) {
			__debug("malloc()" __GM__);
			pup->tf = 0;
			goto out;
		}
		if (0 == (pup->flowout=malloc(pup->tf * sizeof(int)))) {
			__debug("malloc()" __GM__);
			pup->tf = 0;
			assert(pup->flowin);
			free(pup->flowin);
			goto out;
		}
		pup->efr = 1;
		__ndebug("pup->efr == %u\n" _ pup->efr);
		memset(pup->flowin, 0, pup->tf*sizeof(int));
		memset(pup->flowout, 0, pup->tf*sizeof(int));
	}
	assert(pup->tf);
	assert(pup->flowin);
	assert(pup->flowout);
	
out:
	ret = 0;
err:
	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static unsigned char *_get_val_int(int ind, unsigned char *buf, struct up *pup,
	unsigned char *hunt, unsigned int *hl)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int vl = 0;

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl))
		return 0;

	if (T_OID != p[i++]) {
		__debug(__GM__);
		return 0;
	}
	if (0 >= cklen(p, &i, &vl))
		return 0;
	
	for (j=0; j<_bind[ind].len; j++) {
		if (0 == memcmp(&p[i], _bind[ind].bind+j*(ZHZ_GETITM)+4, 8))
			break;
	}
	if (_bind[ind].len == j) {
		__debug(__GM__);
		return 0;
	}
	i += 8;

	__ndebug("j == %d, type == %d\n" _ j _ p[i]);
	if (T_INT!=p[i] && T_COUNT!=p[i] && T_GAUGE!=p[i]) {
		__debug("type error" __GM__);
		return 0;
	} 
	i++;
	if (-1 == cklen(p, &i, &vl))
		return 0;
	
	{	
		int jj, rsl = 0;
		unsigned int l = 0;
		
		for (jj=0, l=0; jj<vl; jj++) {
			l = (l<<8) + p[i+jj];
		}
		rsl = sprintf(hunt+11*j, "%u", l);
		assert(10 >= rsl);
		*(hl+j) = rsl + 1;
	}

	return p + i + vl;
}

static int get_val_int(unsigned char *buf, unsigned int tl, 
	struct up *pup, struct probe *top, unsigned int ind)
{
	unsigned char *p = buf;
	unsigned int i = 0;
	unsigned int l = 0;
	unsigned char *hunt = 0;
       	unsigned int *hl = 0;
	int ret = 2;
	
	assert(0<ind && (ALL_BIND-1)>ind);
	
	if (0 == (hunt=calloc(_bind[ind].len, 11)))
		__edebug("calloc()" __GM__);
	if (0 == (hl=calloc(_bind[ind].len, 4)))
		__edebug("calloc()" __GM__);
		
	for (i=0; i<_bind[ind].len; i++) {
		p = _get_val_int(ind, p, pup, hunt, hl);
		if (0 == p) {
			__debug("return error" __GM__);
			goto err;
		}
		if (tl < p - buf) {
			__debug("length error" __GM__);
			goto err;
		}
	}
	
	if (tl != p - buf) {
		__debug("length error" __GM__);
		goto err;
	}

	for (i=0; i<_bind[ind].len; i++) {
		int sl = 0;
		
		if (0 == (sl=strlen(hunt+11*i))) {
			__debug("one is NULL" __GM__);
			goto err;
		}
		if (1 >= *(hl+i)) {
			__debug("STRAN  " __GM__);
			goto err;
		}
		assert(sl+1 == *(hl+i));
		l += sl + 1;
	}
	
#ifdef __INT_DEBUG__
	//if (ICMP_BIND == ind) {
	__debug("INT -- <%d>\n" _ ind);
	for (i=0; i<_bind[ind].len; i++)
		__debug("\t(%d)==(%s)\n" _ i _ hunt+11*i);
	__debug("INT -- <%d> END\n" _ ind);
	//}
#endif
	
#ifdef __SEND_TRUE__ 
#endif

	if (IP_BIND == ind) {
		pup->isgw = atoi(hunt);
		if (1!=pup->isgw && 2!=pup->isgw)
			goto err;
		/////////////////////////////////////////////
		if (1==pup->isgw && 1<pup->ifnum) {
			if (0 == pup->enr) {
				assert(0 == pup->nreq);
				pup->nreq = zhz_getreq(_next[IPAD_NEXT].bind, _next[IPAD_NEXT].len*ZHZ_GETNEXT_ITM, PT_GETNEXTREQ, 0);
				pup->enr = 1;
			} else 
				assert(pup->nreq);
		} else {
			if (pup->nreq) {
				assert(pup->enr);
				if (pup->nreq->req)
					free(pup->nreq->req);
				free(pup->nreq);
				pup->nreq = 0;
				pup->enr = 0;
			} else {
				assert(0 == pup->enr);
			}
		}
		/////////////////////////////////////////////
	}
	
	ret = 0;

err:
	free(hunt);
	free(hl);
	
	__ndebug("out of get_val_int %d\n" _ ind);
	return ret;
}

//////////////////////////////////////////////////////////////////////////////

static unsigned char *_getnext_val(unsigned char *buf, struct up *pup,
	unsigned char **ifp, int *ifl, int *is_end, struct ifinfo *ifi)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	unsigned int l = 0, vl = 0, vll = 0;

	assert(pup && buf && ifp && ifl);
	assert(ifi && is_end);
	assert(list_empty(&ifi->ifinfo));
	
	__ndebug("into _getnext_val()\n");
	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		goto out;
	}
	if (0 >= cklen(p, &i, &vl))
		goto out;

	if (T_OID != p[i++]) {
		__debug(__GM__);
		goto out;
	}
	if (0 >= cklen(p, &i, &vl))
		goto out;
	for (j=0; j<ZHZ_IPAD_NEXT_NUM; j++) {
		if (0 == memcmp(p+i, _next[IPAD_NEXT].bind+j*(ZHZ_GETNEXT_ITM)+4, 9))
			break;
	}
	if (ZHZ_IPAD_NEXT_NUM == j) {
		unsigned char end[] = {0x2b,0x6,0x1,0x2,0x1,0x4,0x15,0x1,0x1};
	
		if (memcmp(p+i, end, 9)) {
			__debug(__GM__);
			goto out;
		}
		__ndebug("end of mib\n");
		*is_end = 1;
		goto out;
	}

	__ndebug("j == %d, type == %d\n" _ j _ p[i+vl]);
	if (T_IPADDR != p[i+vl]) {
	       	if (0==j || 2==j) {
			__debug("type error" __GM__);
			goto out;
		}	
	}

	if (T_INT != p[i+vl]) {
		if (1==j || 3==j || 4==j) {
			__debug("type error" __GM__);
		       	goto out;
		}
	}
	
	if (0 == (ifp[j]=malloc(vl+6)))
		__edebug("malloc()" __GM__);
	ifp[j][0] = T_SEQ;
	ifp[j][1] = vl+4;
	ifp[j][2] = T_OID;
	ifp[j][3] = vl;
	memcpy(ifp[j]+4, p+i, vl);
	ifp[j][4+vl] = T_NULL;
	ifp[j][4+vl+1] = 0x0;
	ifl[j] = vl + 6;
	__ndebug("ifl[%d] == %d\n" _ j _ ifl[j]);
	i += vl;

	i++;
	
	if (-1 == cklen(p, &i, &vll))
		goto err;
	
	if (1 == j) {
		int jj;
		for (jj=0, l=0; jj<vll; jj++)
			l = (l<<8) + p[i+jj];
		ifi->ipifindex = l;
	} else if(0==j || 2==j) {
		__ndebug("%d\n" _ vll);
		if (4 != vll)
			goto err;
		memcpy(0==j?ifi->ipaddr:ifi->ipmask, p+i, 4);
	} else if (3==j) {
		int jj;
		for (jj=0, l=0; jj<vll; jj++)
			l = (l<<8) + p[i+jj];
		ifi->ipbcast = l;
	} else if ( 4==j) {
		int jj;
		for (jj=0, l=0; jj<vll; jj++)
			l = (l<<8) + p[i+jj];
		ifi->ipmaxsize = l;
	} else {
		__debug("out of range" __GM__);
		goto err;
	}
		
	return p + i + vll;

err:
	free(ifp[j]);
	ifp[j] = 0;
	ifl[j] = 0;
out:
	return 0;
}

static void find_probe(unsigned int ad, unsigned int addr, unsigned int mask, 
	struct probe *otop)
{
	struct probe *top = 0, *ttop = 0;
	struct list_head *l;
	
	if (0 == (top=malloc(sizeof(struct probe))))
		__edebug("malloc()" __GM__);
	memset(top, 0, sizeof(struct probe));
	INIT_LIST_HEAD(&top->probes);
	top->addr = addr;
	top->mask = mask;
	top->distant = otop->distant + 1;
	top->state = READY;

	pthread_mutex_lock(&otop->phead->lock);
	for (l=otop->phead->probes.next;l!=&otop->phead->probes;l=l->next) {
		ttop = list_entry(l, struct probe, probes);
		if (0==memcmp(&addr, &ttop->addr, sizeof(unsigned int)) &&
			0==memcmp(&mask, &ttop->mask, sizeof(unsigned int))) {
			pthread_mutex_unlock(&otop->phead->lock);
			free(top);
			return;
		}
	}
	for (l=otop->phead->newprobes.next;l!=&otop->phead->newprobes;l=l->next) {
		ttop = list_entry(l, struct probe, probes);
		if (0==memcmp(&addr, &ttop->addr, sizeof(unsigned int))/* &&
			0==memcmp(&mask, &ttop->mask, sizeof(unsigned int))*/) {
			pthread_mutex_unlock(&otop->phead->lock);
			free(top);
			return;
		}
	}
	__debug("add new probe +++++< ADDR:%d.%d.%d.%d\tMASK:%d.%d.%d.%d >\n" _
		((unsigned char *)(&addr))[0] _ ((unsigned char *)(&addr))[1] _ 
		((unsigned char *)(&addr))[2] _ ((unsigned char *)(&addr))[3] _ 
		((unsigned char *)(&mask))[0] _ ((unsigned char *)(&mask))[1] _ 
		((unsigned char *)(&mask))[2] _	((unsigned char *)(&mask))[3]);
	list_add(&top->probes, &otop->phead->newprobes);
	sem_post(&otop->phead->sem);
	pthread_mutex_unlock(&otop->phead->lock);
}

#ifdef __SEND_TRUE__
#endif

static int getnext_val(unsigned char *buf, unsigned int tl, struct up *pup, 
	struct probe *top)
{
	unsigned char *p = buf;
	unsigned int j;
	unsigned char *ifp[ZHZ_IPAD_NEXT_NUM] = {0, 0, 0, 0, 0};
	int ifl[ZHZ_IPAD_NEXT_NUM] = {0, 0, 0, 0, 0};
	int is_end = 0;
	struct ifinfo *ifi = 0;

	__ndebug("into getnext_val()\n");
	if (0 == (ifi=malloc(sizeof(struct ifinfo))))
		__edebug("malloc()" __GM__);
	memset(ifi, 0, sizeof(struct ifinfo));
	INIT_LIST_HEAD(&ifi->ifinfo);
	
	for (j=0; j<ZHZ_IPAD_NEXT_NUM; j++) {
		p = _getnext_val(p, pup, ifp, ifl, &is_end, ifi);
		if (is_end) {
			struct list_head *l = 0;
			struct ifinfo *tifi = 0;
			unsigned int cgi = 0;
			
			assert(0 == p);
			assert(pup->nreq && pup->nreq->req);
			free(pup->nreq->req);
			free(pup->nreq);
			pup->nreq = 0;
			pup->enr = 0;
#ifdef __SEND_TRUE__ 
#else
			for (l=pup->ifinfo.next;l!=&pup->ifinfo;l=l->next) {
				tifi = list_entry(l, struct ifinfo, ifinfo);
				l = l->prev;
				list_del_init(&tifi->ifinfo);
				free(tifi);
			}
#endif	
			assert(list_empty(&pup->ifinfo));
			goto err;
		}
		if (0 == p) {
			__debug("return error" __GM__);
			goto err;
		}
		if (tl < p - buf) {
			__debug("length error" __GM__);
			goto err;
		}
	}
	
	if (tl != p - buf) {
		__debug("length error" __GM__);
		goto err;
	}

	assert(pup->nreq && pup->nreq->req);
	free(pup->nreq->req);
	free(pup->nreq);
	pup->nreq = 0;
	pup->enr = 0;

	if (0==ifp[0] || 0==ifp[1] || 0==ifp[2] || 0==ifp[3] || 0==ifp[4]) {
		if (ifp[0] || ifp[1] || ifp[2] || ifp[3] || ifp[4]) {
			__debug(__GM__);
			goto err;
		}
	}
	
	if (!LOOPBACK(*((unsigned int *)ifi->ipaddr)) &&
		!MULTICAST(*((unsigned int *)ifi->ipaddr)) && 
		!ZERONET(*((unsigned int *)ifi->ipaddr)) &&
		!BADCLASS(*((unsigned int *)ifi->ipaddr))) {
		struct list_head *l = 0;
		struct ifinfo *tifi = 0;
		unsigned int na, oa, nn, nm, om, on;
		unsigned int all1;
		
		{
			unsigned char *__p__ = (unsigned char *)&all1;
			int __i__;
			for (__i__=0; __i__<4; __i__++)
				__p__[__i__] = 255;
		}
		
		memcpy(&na, (unsigned int *)ifi->ipaddr, sizeof na);
		memcpy(&nm, (unsigned int *)ifi->ipmask, sizeof nm);
		nn = (na & nm);

		if (0 == memcmp(&nm, &all1, sizeof nm)) 
			goto clean_not_new;

		for (l=pup->ifinfo.next;l!=&pup->ifinfo;l=l->next) {
			tifi = list_entry(l, struct ifinfo, ifinfo);
			memcpy(&oa, (unsigned int *)tifi->ipaddr, sizeof oa);
			memcpy(&om, (unsigned int *)tifi->ipmask, sizeof om);
			on = (oa & om);
			
			if (0==memcmp(&nn, &on, sizeof nn) &&
				0==memcmp(&nm, &om, sizeof nm)) 
				goto clean_not_new;
		}	
#ifdef __SEND_TRUE__
#endif
		__ndebug("find --> addr=%d.%d.%d.%d, mask=%d.%d.%d.%d, "
			"index=%d\n" _ 
			ifi->ipaddr[0] _ ifi->ipaddr[1] _ ifi->ipaddr[2] _
			ifi->ipaddr[3] _ ifi->ipmask[0] _ ifi->ipmask[1] _
	       	 	ifi->ipmask[2] _ ifi->ipmask[3] _ ifi->ipifindex);
		list_add(&ifi->ifinfo, &pup->ifinfo);

		if (0==memcmp(&nn, &top->addr, sizeof nn) &&
			0==memcmp(&nm, &top->mask, sizeof nm)) 
			goto not_new;

		if (PRINET1(*((unsigned int *)ifi->ipaddr))/* ||
			PRINET2(*((unsigned int *)ifi->ipaddr))*/) 
			goto not_new;

		find_probe(na, nn, nm, top);

		goto not_new;
	}

clean_not_new:
	free(ifi);
not_new:
	ifi = 0;

	if (0 == ifp[0]) {
		assert(is_end);
		pup->enr = 0;
	} else {
		unsigned char *tp = 0;
		unsigned int tl = 0;
		
		assert(0 == is_end);
		if (0 == (tp=malloc(ifl[0] + ifl[1] + ifl[2] + ifl[3] + ifl[4])))
			__edebug("malloc()" __GM__);
		for (j=0; j<ZHZ_IPAD_NEXT_NUM; j++) {
			memcpy(tp+tl, ifp[j], ifl[j]);
			__ndebug("ifl[%d] == %d\n" _ j _ ifl[j]);
			tl += ifl[j];
			free(ifp[j]);
		}
		__ndebug("tl == %d\n" _ tl);
		pup->nreq = zhz_getreq(tp, tl, PT_GETNEXTREQ, 0);
		free(tp);
		pup->enr = 1;
	}
	
	return 0;

err:
	for (j=0; j<ZHZ_IPAD_NEXT_NUM; j++) {
		if (ifp[j])
			free(ifp[j]);
	}
	free(ifi);
	
	return is_end ? 0 : 2;
}

////////////////////////////////////////////////////////////////////////////// 

int prase_resp(unsigned char *buf, struct up *pup, struct probe *top, 
	int all, int ind)
{
	unsigned char *p = buf;
	unsigned int i = 0, j;
	int l = 0, idl = 0;
	unsigned int sl = 0, pl = 0, vl = 0, ool = 0, comml = 0;
	unsigned char *start = 0;
	int _comml;
	int verl = 0;

	if (DO_GETNEXT_REQ == all) {
		if (!(pup->enr && pup->nreq)) {
			__debug("Something ugly happen" __GM__);
			return 1;
		}
	}

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &sl))
		return 2;
	
	start = p + i;
	
	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == (verl=cklen(p, &i, &ool)))
		return 2;

	if (1 != ool) {
#ifdef __ZHZ_DEBUG__
		assert(0);
#endif
		__debug(__GM__);
		return 2;
	}
	if (SNMP_VER1 != p[i++]) {
		__debug(__GM__);
		return 2;
	}

	if (T_STR != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == (comml=cklen(p, &i, &ool)))
		return 2;

	if(-1 == (_comml=check_comm(&p[i], ool)))
		return 2;
	i += _comml;
	
	if (PT_GETRESP != p[i]) {
		if (PT_TRAP!=p[i++] || DO_TRAP!=all) {
			__debug(__GM__);
			return 2;
		} else {
			if (-1 == (l=cklen(p, &i, &pl)))
				return 2;
			return get_trap(p+i, pl, buf);
		}
	} 
	i++;
	
	assert(DO_TRAP != all);

	if (-1 == (l=cklen(p, &i, &pl)))
		return 2;

	if (pl+(LEN_ID<l?(l-LEN_ID):0)+(LEN_ID<comml?(comml-LEN_ID):0)+
		(LEN_ID<verl?(verl-LEN_ID):0)+2+5+_comml != sl) {
#ifdef __ZHZ_DEBUG__
		__debug("%d - %d = %d\n" _ sl _ pl _ (sl-pl));
		assert(0);
#endif
		__debug(__GM__);
		return 2;
	}
	
	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == (verl=cklen(p, &i, &idl)))
		return 2;
	{
		unsigned char *ip = &p[i];
		unsigned int xx, ss = 0, tt = 0;

		for (xx=0; xx<idl; xx++) 
			ss = (ss<<8) + p[i+xx];
		if (DO_IF_REQ == all)
			for (xx=0; xx<top->sr[IF_BIND]->req_id[1]; xx++)
				tt = (tt<<8) + top->sr[IF_BIND]->req_id[2+xx];
		else if(DO_FLOW_REQ == all) 
			for (xx=0; xx<top->sr[FLOW_BIND]->req_id[1]; xx++)
				tt = (tt<<8) + top->sr[FLOW_BIND]->req_id[2+xx];
		else if (DO_GETNEXT_REQ == all) {
			int qi;
			unsigned char *qp = (unsigned char *)&top->nextid;
			for (qi=0; qi<4; qi++)
				tt = (tt<<8) + qp[qi];
		} else
			for (xx=0; xx<top->sr[ind]->req_id[1]; xx++)
				tt = (tt<<8) + top->sr[ind]->req_id[2+xx];
		if (ss != tt) {
			return 3;
		}
		i += idl;
	}

	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &ool))
		return 2;

	if (1>ool || 4<ool) {
		__debug(__GM__);
		return 2;
	}
	for (j=0; j<ool; j++) {
		if (p[i+j]) {
			if (1 == p[i+j])
				__debug("TooBig ERROR: l == %d, j == %d\n" _
					l _ j);
			__ndebug(__GM__);
			return 3;
		}
	}
	i += ool;
	
	if (T_INT != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &ool))
		return 2;
	
	if (1>ool || 4<ool) {
		__debug(__GM__);
		return 2;
	}
	for (j=0; j<ool; j++) {
		if (p[i+j]) {
			assert(0);
			__debug(__GM__);
			return 3;
		}
	}
	i += ool;

	if (T_SEQ != p[i++]) {
		__debug(__GM__);
		return 2;
	}
	if (-1 == cklen(p, &i, &vl))
		return 2;

#ifdef __ZHZ_DEBUG__
	assert(p+i-start == sl-vl);
#endif
	if (p+i-start != sl-vl) {
		__debug(__GM__);
		return 2;
	}
	
	if (DO_GETNEXT_REQ == all)
		return getnext_val(p+i, vl, pup, top);
	else if (DO_FLOW_REQ == all)
		return get_val_flow(p+i, vl, pup, top);
	else if (DO_GET_REQ == all) {
		if (SYS_BIND == ind)
			return get_val_sys(p+i, vl, pup, top);
		else 
			return get_val_int(p+i, vl, pup, top, ind);
	} else if (DO_IF_REQ == all)
		return get_val_if(p+i, vl, pup, top);
}

static int rcv_resp(int fd, struct probe *top, int all, int ind, struct up **up)
{
	struct sockaddr_in addr;
	int l = sizeof addr;
	int r;
	unsigned char buf[65536];
	struct up *pup = 0;
	
	assert(top);
	*up = pup;
	memset(&addr, 0, sizeof addr);
	memset(buf, 0, sizeof buf);
	__ndebug("something comming!\n");

	r = recvfrom(fd, buf, sizeof buf, 0, (struct sockaddr *)&addr, &l);
	if (-1 == r)
		__edebug("recvfrom()" __GM__);
	if (l != sizeof addr) {
		__debug("wrong sockaddr length!\n");
		return 1;
	}
#ifdef __ZHZ_DEBUG__
	{
	unsigned char *p = 0;
	p = (unsigned char *)&addr.sin_addr.s_addr;
	__ndebug("recvfrom resp from %d.%d.%d.%d:%d\n" _ p[0] _ p[1] _ 
			p[2] _ p[3] _ ntohs(addr.sin_port));
	}
#endif
	if (PORT_SNMP != ntohs(addr.sin_port))
		return 1;
	if (0 == (pup=ckaddr(&addr, top)))
		return 1;
	*up = pup;
	
	return prase_resp(buf, pup, top, all, ind);
}

///////////////////////////////////////////////////////////////////////////////

int do_snmp(struct probe *top, int all, int ind, int repeat)
{
	int k = 0, i;
	struct sockaddr_in addr;
	struct pollfd pfd[2];
	struct list_head *l = 0;
	struct up *pup = 0;
	int is_end = 1;
	int fd = top->fds;
	
	pfd[0].fd = pfd[1].fd = fd;
	pfd[0].events = POLLIN;
	pfd[1].events = POLLOUT;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT_SNMP);
	
	__ndebug("$$$$$$$$$$$$$$$$$$$    in do_snmp()     $$$$$$$$$$$$$$$$$\n");

	START_ROLL
	poll(pfd, 2, 10000);
	if (POLLIN & pfd[0].revents) {
		struct up *up = 0;
		
		switch (rcv_resp(fd, top, all, ind, &up)) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			__ndebug("bug 1\n");
			break;
		default:break;
		}
		if (DO_IF_REQ == all) {
			if (up) {
				unsigned char * __p = 
					(unsigned char *)&up->addr;
				__ndebug("RECV...............%d.%d.%d.%d (%d) "
					"<%d>\n" _ __p[0] _ __p[1] _ __p[2] _ 
					__p[3] _ up->eir _ up->ifnum);
				if (up->eir)
					up->eir++;
				if (up->eir > up->ifnum)
					up->eir = 0;
			}
		} else if (DO_FLOW_REQ == all) {
			if (up) {
				unsigned char * __p = 
					(unsigned char *)&up->addr;
				__ndebug("RECV...............%d.%d.%d.%d (%d) "
					"<%d>\n" _ __p[0] _ __p[1] _ __p[2] _ 
					__p[3] _ up->efr _ up->tf);
				up->efr++;
				(up->efr>up->tf) && (up->efr=1);
			}
		}
	} else if (POLLOUT & pfd[1].revents) {
		pup = list_entry(l, struct up, list);
		if (DO_GETNEXT_REQ == all) {
			assert(IPAD_NEXT == ind);
			if (pup->enr) {
				if (pup->ifnum+5 < repeat) {
					assert(pup->nreq && pup->nreq->req);
					free(pup->nreq->req);
					free(pup->nreq);
					pup->nreq = 0;
					pup->enr = 0;
				} else {
					unsigned char * __p = 
						(unsigned char *)&pup->addr;
					assert(pup->nreq);
					send_next(pup, top, &addr);
					is_end = 0;
					__ndebug("...............%d.%d.%d.%d "
						"<%d>\n" _ __p[0] _ __p[1] _ 
						__p[2] _ __p[3] _ pup->ifnum);
				}
			} else
				assert(0 == pup->nreq);
		} else if (DO_GET_REQ == all) {
			is_end = 0;
			if (0 == ind) {
				pup->ifnum = 0;
				pup->isgw = 0;
			}
			send_get(pup, top, &addr, ind);
		} else if (DO_IF_REQ == all) {
			if (ind) {
				if (pup->eir)
					goto there;
				if(pup->ifnum && 1<=pup->ifnum) {
					pup->eir = 1;
					send_if(pup, top, &addr, pup->eir);
					is_end = 0;
				}
			} else if (pup->eir) {
there:
				if (pup->ifnum+5 < repeat) {
					pup->eir = 0;
				} else if(pup->ifnum && pup->eir<=pup->ifnum) {
					unsigned char * __p = 
						(unsigned char *)&pup->addr;
					send_if(pup, top, &addr, pup->eir);
					is_end = 0;
					__ndebug("...............%d.%d.%d.%d "
						"(%d) <%d>\n" _ __p[0] _ 
						__p[1] _ __p[2] _ __p[3] _ 
						pup->eir _ pup->ifnum);
				} else 
					pup->eir = 0;
			} 
		} else if (DO_FLOW_REQ == all) {
			if(pup->tf) {
				unsigned char * __p = 
					(unsigned char *)&pup->addr;
				__ndebug("send flow ...............%d.%d.%d.%d "
					"(%d) <%d>\n" _ __p[0] _ 
					__p[1] _ __p[2] _ __p[3] _ 
					pup->efr _ pup->tf);
			        if (1==pup->efr && 0!=repeat) 
					continue;
				assert(0 < pup->efr);
			       	assert(pup->efr <= pup->tf);
				send_flow(pup, top, &addr, pup->efr);
				is_end = 0;
			} else {
				__ndebug("efr = 0\n");
				pup->efr = 0;
			}
		} else
			assert(0);
	}
	END_ROLL
	
	if (is_end) {
		__ndebug("<<<<<<<<<<<<<<<<<<<<<<<<<<<is_end %d %d "
			">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n" _ all _ ind);
		return 1;
	}
	
	for (;;) {
		int u;
		
		__ndebug("into wait poll()\n");
		u = poll(pfd, 1, 1000*top->distant);
		__ndebug("out of wait poll\n");

		if (0 == u) {
			__ndebug("timeout wait poll, return\n");
			break;
		}
		
		if (POLLIN & pfd[0].revents) {
			struct up *up = 0;
			
			switch (rcv_resp(fd, top, all, ind, &up)) {
			case 0:
				break;
			case 1:
				break;
			case 2:
				__ndebug("bug2 \n");
				break;
			default:break;
			}
			// FIX ME!
			// If one delay packet and resend packet come ......
			if (DO_IF_REQ == all) {
				if (up) {
					unsigned char * __p = 
						(unsigned char *)&up->addr;
					__ndebug("WAIT RECV..............."
						"%d.%d.%d.%d (%d) <%d>\n" _ 
						__p[0] _ __p[1] _ __p[2] _ 
						__p[3] _ up->eir _ up->ifnum);
					if (up->eir)
						up->eir++;
					if (up->eir > up->ifnum)
						up->eir = 0;
				}
			} else if (DO_FLOW_REQ == all) {
				if (up) {
					unsigned char * __p = 
						(unsigned char *)&up->addr;
					__ndebug("RECV..............."
						"%d.%d.%d.%d (%d) <%d>\n" _ 
						__p[0] _ __p[1] _ __p[2] _ 
						__p[3] _ up->efr _ up->tf);
					up->efr++;
					(up->efr>up->tf) && (up->efr=1);
				}
			}
			__ndebug("rcv_resp return\n");
		}
		__ndebug("poll() return error\n");
	}
	__ndebug("out of do_snmp()\n");
	return is_end;
}

//
// end of snmp.c
//

