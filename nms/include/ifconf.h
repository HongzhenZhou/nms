
//
// ifconf.h
//

#ifndef _ZHZ_IFCONF_H_
#define _ZHZ_IFCONF_H_

struct ifi {
	char name[IFNAMSIZ];
	unsigned int addr;
	unsigned int mask;
#if 1
	unsigned distant;
#endif
	struct list_head list;
};

char *loopdev = "lo";

#endif // _ZHZ_IFCONF_H_

//
// end of ifconf.h
//

