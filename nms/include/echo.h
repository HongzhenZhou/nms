
#ifndef _ZHZ_ECHO_H_
#define _ZHZ_ECHO_H_

unsigned int get_key(unsigned int *a, unsigned int *b, unsigned int size);
int send_echo(int fd, struct probe *top, unsigned int *a, struct list_head *l);
int recv_echo(int fd, struct probe *top);
struct up *ckaddr(struct sockaddr_in *addr, struct probe *top);

#endif

