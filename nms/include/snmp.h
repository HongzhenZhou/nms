
//
// snmp.h
//

#ifndef _ZHZ_SNMP_H_
#define _ZHZ_SNMP_H_

enum {
	ZHZ_GETITM=14,
	ZHZ_GETNEXT_ITM=15,
	ZHZ_IF_ITM=16,
	
	ZHZ_SYS_NUM=8,
	ZHZ_IP_NUM=19,
	ZHZ_ICMP_NUM=26,
	ZHZ_TCP_NUM=14,
	ZHZ_UDP_NUM=4,
	ZHZ_SNMP_NUM=28,
	ZHZ_IF_NUM=21,
	ZHZ_FLOW_NUM=2,
	
	ZHZ_IPAD_NEXT_NUM=5
};

enum _snmp_type {
	T_INT=0x02,
	T_STR=0x04,
	T_NULL=0x05,
	T_OID=0x06,
	T_SEQ=0x30,

	T_IPADDR=0x40,
	T_COUNT=0x41,
	T_GAUGE=0x42,
	T_TIME=0x43,
	T_OPAQUE=0x44,

	PT_GETREQ=0xa0,
	PT_GETNEXTREQ=0xa1,
	PT_GETRESP=0xa2,
	PT_SETREQ=0xa3,
	PT_TRAP=0xa4,
	PT_BULKREQ=0xa5,
	PT_INFORMREQ=0xa6,
	PT_TRAPV2=0xa7,
	PT_REPORT=0xa8
};

enum {
	DO_GET_REQ = 0,
	DO_GETNEXT_REQ = 1,
	DO_IF_REQ = 2,
	DO_TRAP = 3,
	DO_FLOW_REQ = 4,
};

struct snmp_bind {
	unsigned char *bind;
	unsigned int len;
};

extern struct snmp_bind _bind[ALL_BIND];
extern struct snmp_bind _next[ALL_NEXT];

//
struct snmp_request {
	unsigned int req_len;
	unsigned char *req;
	unsigned char *req_id;
	unsigned char *vsp;
};

//
int cklen(unsigned char *p, unsigned int *i, unsigned int *ool);
struct snmp_request *zhz_getreq(unsigned char *zhz_bind, size_t bind_size,
	unsigned char REQ, int vsp);
struct snmp_request *reuse_getreq(struct snmp_request *, unsigned char *);
int do_snmp(struct probe *top, int all, int ind, int repeat);
void snmp_init();
int prase_resp(unsigned char *buf, struct up *pup, struct probe *top, 
	int all, int ind);

#endif // _ZHZ_SNMP_H_

//
// end of snmp.h
//

