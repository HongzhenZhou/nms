
//
// debug.h
//

#ifndef _ZHZ_DEBUG_H_
#define _ZHZ_DEBUG_H_

#define __GM__ " in file %s, line %d\n", __FILE__, __LINE__

#define __ndebug(s)  

#define __eedebug(s) if (1) {\
	printf("%d >>>> ", getpid());\
	printf(s);\
	fflush(0);\
	assert(0);\
	exit(1);\
} else

#ifdef __ZHZ_DEBUG__
#define __debug(s) if (1) {\
	printf("%d >>>> ", getpid());\
	printf(s);\
	fflush(0);\
} else

#define __odebug(s) if (1) {\
	printf("%d <<<< ", getpid());\
	printf(s);\
	fflush(0);\
} else

#define __edebug(s) if (1) {\
	printf("%d >>>> ", getpid());\
	printf(s);\
	fflush(0);\
	assert(0);\
	exit(1);\
} else

#else
#define __debug(s) 

#define __edebug(s) if (1) {\
	assert(0);\
	exit(1);\
} else

#endif // __ZHZ_DEBUG__

#define _ ,
		
enum {
	SYS_BIND = 0,
	IP_BIND = 1,
	ICMP_BIND = 2,
	TCP_BIND = 3,
	UDP_BIND = 4,
	SNMP_BIND = 5,
	IF_BIND = 6,
	FLOW_BIND = 7,
	ALL_BIND = 8
};

enum {
	IPAD_NEXT = 0,
	ALL_NEXT = 1
};

enum {
	SNET_LEN=18,
	ADDR_LEN=22,
	DEPT_LEN=10,
	TYPE_LEN=10
};

enum {
	DSTAT_OK=1,
	DSTAT_DEL=2,
	DSTAT_SICK=0
};

#endif // _ZHZ_DEBUG_H_

//
// end of debug.h
//

