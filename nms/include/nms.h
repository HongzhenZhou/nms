
//
// nms.h
//

#ifndef _ZHZ_NMS_H_
#define _ZHZ_NMS_H_

// lock files pathname
#define D_LOCK "/var/lock/zhznm"
#define F_LOCK_ZHZ "/var/lock/zhznm/nms"

// lock file open flag & mode
#define FMODE S_IRUSR|S_IWUSR
#define OFLAG O_RDWR|O_CREAT

//
#define CF_NEINMS "/etc/nei-nms.conf"
#define CP_NEINMS "/usr/local/sbin/nei-nmsc.sh"

//
#define PIDLEN 9

#endif // _ZHZ_NMS_H_

//
// end of nms.h
//

