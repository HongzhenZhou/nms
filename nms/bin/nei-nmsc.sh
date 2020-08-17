#!/bin/bash

NEINMSCONF="/etc/nei-nms.conf"

trap "" 1 2 3 15
[ '0' != `/usr/bin/id -u` ] && { echo "You are NOT root!"; exit 1; }
trap 1 2 3 15

dept=
type=
raddr=
port=

. ./drawlib.sh

tux_dark( ) {
	local SURE_BCOLOR=41; local SURE_FCOLOR=32
	local nlength=`echo $1 | wc -c`; local sure=""
	set_color $SURE_FCOLOR $SURE_BCOLOR
	draw_squa $((${SCR_LENS}/2-1)) $((${SCR_COLS}/2-25)) 3 50 $bcolor
	echo -en "[1;${fcolor}m"
	##tput $((${SCR_LENS}/2+1)) $((${SCR_COLS}/2
	echo -en "[$((${SCR_LENS}/2+1));$((${SCR_COLS}/2-20))H"
	#tput blink; 
	echo -n "$*"; tput sgr0 
	echo -en "[1;${bcolor}m[1;${fcolor}m" 
	tput cnorm 
	set_color $old_fcolor $old_bcolor
}

read_conf( ) {
	local tt
	for tt; do
		tux_dark $tt " = "
		read -e -n 35 $tt
		reset_scr
		clear
		stty -echo
		tux_dark "                   OK"
		sleep 1
		stty echo
	done
}

reset_scr
clear
init_scr_set
init_scr_const

trap "stty echo" 1 2 3 15 
read_conf dept type raddr rport
reset_scr
clear
trap 1 2 3 15

trap "" 1 2 3 15 
echo "dept" $dept > $NEINMSCONF
echo "type" $type >> $NEINMSCONF
echo "raddr" $raddr >> $NEINMSCONF
echo "rport" $rport >> $NEINMSCONF
trap 1 2 3 15

exit 0

