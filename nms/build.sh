
#!/bin/bash

usage () {
	echo "USAGE: $1 0|1|2|3|4|5|7"
	sleep 1
	echo "default: $1 7"
	sleep 1
	echo "do $1 7"
	sleep 1
}

case $1 in
0) make clean;;
1) make DFLAGS='-D__ZHZ_DEBUG__';;
2) make DFLAGS='-D__ZHZ_DEBUG__ -D__ZHZ_DEBUG_1__';;
3) make DFLAGS='-D__ZHZ_DEBUG__ -D__ZHZ_DEBUG_1__ -D__SYS_DEBUG__';;
4) make DFLAGS='-D__ZHZ_DEBUG__ -D__ZHZ_DEBUG_1__ -D__IF__DEBUG__';;
5) make DFLAGS='-D__ZHZ_DEBUG__ -D__ZHZ_DEBUG_1__ -D__INT_DEBUG__';;
6) make DFLAGS='-D__ZHZ_DEBUG__ -D__ZHZ_DEBUG_1__ -D__SYS_DEBUG__ -D__IF__DEBUG__ -D__INT_DEBUG__';;
7) make DFLAGS='-D__ZHZ_DEBUG__ -D__SYS_DEBUG__ -D__IF__DEBUG__ -D__INT_DEBUG__';;
*) usage $0
   make DFLAGS='-D__ZHZ_DEBUG__ -D__SYS_DEBUG__ -D__IF__DEBUG__ -D__INT_DEBUG__';;
esac


