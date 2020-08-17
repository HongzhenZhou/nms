
#include <assert.h>
#define __USE_UNIX98
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <asm/types.h>
#include <linux/icmp.h>

#ifdef __SEND_TRUE__ 
#endif

#include "debug.h"
#include "list.h"
#include "probe.h"
#include "send.h"
#include "lock.h"

//////////////////////////////////////////////////////////////////////////////

static unsigned char community[257] = {'p', 'u', 'b', 'l', 'i', 'c', '\0',};
static pthread_rwlock_t comm_lock = PTHREAD_RWLOCK_INITIALIZER;

unsigned int check_comm(unsigned char *p, unsigned int ool)
{
	int _comml;

	pthread_rwlock_rdlock(&comm_lock);
	_comml = strlen(community);
	if (0 == _comml)
		return 0;
	if (_comml != ool) {
		pthread_rwlock_rdlock(&comm_lock);
		__debug(__GM__);
		return 0;
	}
	if (strncmp(p, community, _comml)) {
		pthread_rwlock_rdlock(&comm_lock);
		__debug(__GM__);
		return 0;
	}
	pthread_rwlock_rdlock(&comm_lock);
	
	return _comml;
}

int get_comm(unsigned char *comm, unsigned int *l)
{
	pthread_rwlock_rdlock(&comm_lock);
	*l = strlen(community);
	memcpy(comm, community, *l+1);
	pthread_rwlock_unlock(&comm_lock);

	return 0;
}

int set_comm(unsigned char *comm, unsigned int l)
{
	pthread_rwlock_wrlock(&comm_lock);
	assert(l == strlen(comm) + 1);
	memcpy(community, comm, l);
	pthread_rwlock_unlock(&comm_lock);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////

static time_t gmt = 600;
static pthread_rwlock_t gmt_lock = PTHREAD_RWLOCK_INITIALIZER;

time_t get_gmt()
{
	time_t _gmt;
	
	pthread_rwlock_rdlock(&gmt_lock);
	_gmt = gmt;
	pthread_rwlock_unlock(&gmt_lock);

	return _gmt;
}

void set_gmt(time_t _gmt)
{
	pthread_rwlock_wrlock(&gmt_lock);
	if (_gmt < 600)
		gmt = 600;
	else
		gmt = _gmt;
	pthread_rwlock_unlock(&gmt_lock);
}

//////////////////////////////////////////////////////////////////////////////

static unsigned int ism = 20;
static pthread_rwlock_t ism_lock = PTHREAD_RWLOCK_INITIALIZER;

unsigned int get_ism()
{
	unsigned int _ism;
	
	pthread_rwlock_rdlock(&ism_lock);
	_ism = ism;
	pthread_rwlock_unlock(&ism_lock);

	return _ism;
}

void set_ism(unsigned _ism)
{
	unsigned int _isd;

	_isd = get_isd();
	
	pthread_rwlock_rdlock(&ism_lock);
	if (_isd > _ism)
		ism = _ism;
	pthread_rwlock_unlock(&ism_lock);
}

//////////////////////////////////////////////////////////////////////////////

static unsigned int isd = 20;
static pthread_rwlock_t isd_lock = PTHREAD_RWLOCK_INITIALIZER;

unsigned int get_isd()
{
	unsigned int _isd;
	
	pthread_rwlock_rdlock(&isd_lock);
	_isd = isd;
	pthread_rwlock_unlock(&isd_lock);

	return _isd;
}

void set_isd(unsigned int _isd)
{
	unsigned int _ism;

	_ism = get_ism();
	
	pthread_rwlock_rdlock(&isd_lock);
	if (_isd > _ism)
		isd = _isd;
	pthread_rwlock_unlock(&isd_lock);
}

//////////////////////////////////////////////////////////////////////////////

static time_t silent = 23;
static pthread_rwlock_t silent_lock = PTHREAD_RWLOCK_INITIALIZER;

unsigned int get_silent()
{
	unsigned int _silent;
	
	pthread_rwlock_rdlock(&silent_lock);
	_silent = silent;
	pthread_rwlock_unlock(&silent_lock);

	return _silent;
}

void set_silent(unsigned _silent)
{
	pthread_rwlock_rdlock(&silent_lock);
	if (5 > _silent)
		silent = 5;
	else
		silent = _silent;
	pthread_rwlock_unlock(&silent_lock);
}

//////////////////////////////////////////////////////////////////////////////


