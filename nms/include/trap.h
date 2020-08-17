
//
// trap.h
//

#ifndef _ZHZ_TRAP_H_
#define _ZHZ_TRAP_H_

int get_trap(unsigned char *buf, unsigned int tlen, 
	unsigned char *start);
void *do_trap(void *noop);

#endif // _ZHZ_TRAP_H_

//
// end of trap.h
//

