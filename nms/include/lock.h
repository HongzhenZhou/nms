

#ifndef _ZHZ_LOCK_H_
#define _ZHZ_LOCK_H_

unsigned int check_comm(unsigned char *p, unsigned int ool);
int get_comm(unsigned char *comm, unsigned int *l);
int set_comm(unsigned char *comm, unsigned int l);
time_t get_gmt();
void set_gmt(time_t _gmt);
unsigned int get_isd();
void set_isd(unsigned _isd);
unsigned int get_silent();
void set_silent(unsigned _silent);

#endif 

