#!/bin/bash
#set -x
##
##                 ---------------> X
##                |
##                |
##                |
##                |               
##                |               always Y first, then X
##                V Y
##

##
## general global variable:
##	SCR_LENS	SCR_COLS
##	old_bcolor	old_fcolor
##	bcolor		fcolor
##

## $1 for foreground color, $2 for backgroud color
## color set to 0 for unchange
##
set_color( ) {
	[ $# -ne 2 ] && exit 1
	case $1 in
	0 ) ;; 
	* ) old_fcolor=$fcolor; fcolor=$1;;
	esac
	case $2 in
	0 ) ;;
	* ) old_bcolor=$bcolor; bcolor=$2;;
	esac
}

## $1 Y, $2 X
get_bcolor( ) {
	local curr_color=0
	case $(((${1}+${2})%2)) in
	0 ) let curr_color=$MAGIC_BCOLOR1;;
	1 ) let curr_color=$MAGIC_BCOLOR2;;
	* ) exit 1;;
	esac
	echo $curr_color
	## note!
	return $curr_color
}

## $5 background color
draw_squa( ) {
	[ $# -ne 5 ] && exit 1
	local ystart=$1; local xstart=$2; local ylen=$3; local xlen=$4
	set_color 0 $5
        ##tput cup $ystart $xstart
	echo -en "[$((${ystart}+1));$((${xstart}+1))H"
	echo -en "[0;${bcolor}m"
        local i=1; local j=1
	while [ $i -le $ylen ]; do 
		while [ $j -le $xlen ]; do echo -n " "; let j=$j+1; done
		##tput cup $((${ystart}+${i})) $xstart
		echo -en "[$((${ystart}+${i}+1));$((${xstart}+1))H"
		let i=$i+1; let j=1
	done
	set_color 0 $old_bcolor
}

## $1 Y square moveto, $2 X square moveto,
## $3 foreground color, $4 string, $5 n chars in $4,
## $6 is restore to erase blink
##
draw_man( ) {
	[ $# -ne 6 ] && exit 1;	[ $5 -gt $MAGIC_NUMS ] && exit 1
	local curr_color=`get_bcolor $1 $2`
	[ $6 -ne 0 ] && draw_man $olde_y $olde_x $olde_fcolor \
	"$olde_string" $olde_n 0
	[ $6 -ne 0 ] && { let olde_y=$1; let olde_x=$2; let olde_fcolor=$3; \
	olde_string=$4; let olde_n=$5; }
	draw_squa $((${MAGIC_LENS}*(${1}-1))) $((${MAGIC_COLS}*(${2}-1))) \
	$MAGIC_LENS $MAGIC_COLS $curr_color
	local xstart=$((${MAGIC_COLS}*(${2}-1)+(${MAGIC_COLS}-${5})/2))
	local ystart=$((${MAGIC_LENS}*(${1}-1)+${MAGIC_LENS}/2))
	##tput cup $ystart $xstart
	echo -en "[$((${ystart}+1));$((${xstart}+1))H"
	set_color $3 0
	echo -en "[1;${fcolor}m"; [ $6 -ne 0 ] && tput blink
	echo -n $4; tput sgr0
	set_color $old_fcolor 0
	##tput bel
	[ $6 -ne 0 ] && echo -en "\a"
}

draw_null( ) {
	[ $# -ne 2 ] && exit 1
	local curr_color=`get_bcolor $1 $2`
	draw_squa $((${MAGIC_LENS}*(${1}-1))) $((${MAGIC_COLS}*(${2}-1))) \
	$MAGIC_LENS $MAGIC_COLS $curr_color
}

## erase bottem line and write new stuff
## $1 forecolor, $2- my stuff
##
erase_bottem( ) {
	##local j=1
	##tput cup $SCR_LENS 0
	echo -en "[$((${SCR_LENS}+1));1H"
	set_color 0 40
	echo -en "[1;${bcolor}m"
	##while [ $j -le $SCR_COLS ]; do echo -n " "; let j=$j+1; done
	echo -en "[K"
	[ $# -eq 0 ] && return
	set_color $1 0
	echo -en "[1;${fcolor}m"; shift
	##tput cup $SCR_LENS 0
	echo -en "[$((${SCR_LENS}+1));1H"
	##tput blink; 
	echo -n "$*" " "
	##tput sgr0
	set_color $old_fcolor $old_bcolor
}

## draw chess board
draw_board( ) {
	local curr_color="0"
	local i=0; local j=0
	while [ $i -lt $MAGIC_NUMS ]; do 
		while [ $j -lt $MAGIC_NUMS ]; do
			curr_color=`get_bcolor $i $j`
			draw_squa $((${MAGIC_LENS}*${i})) \
				$((${MAGIC_COLS}*${j})) \
				$MAGIC_LENS $MAGIC_COLS $curr_color
			let j=$j+1
		done; let i=$i+1; let j=0
	done
}

over( ) {
	[ $# -ne 1 ] && exit 1
	local SURE_BCOLOR=41; local SURE_FCOLOR=32; local winner=""
	/usr/bin/clear
	set_color $SURE_FCOLOR $SURE_BCOLOR
	draw_squa $((${SCR_LENS}/2-1)) $((${SCR_COLS}/2-15)) 3 30 $bcolor
	echo -en "[1;${fcolor}m"
	##tput $((${SCR_LENS}/2+1)) $((${SCR_COLS}/2
	echo -en "[$((${SCR_LENS}/2+1));$((${SCR_COLS}/2-10))H"
	if [ 1 -eq $1 ] ; then winner="win"; else winner="lose"; fi
	echo -n "Game is over, you" "${winner}!"
	set_color $old_fcolor 40
	sleep 3
	kill -s 15 $mychild1; kill -s 15 $mychild2
}

are_you_sure( ) {
	local SURE_BCOLOR=41; local SURE_FCOLOR=32
	local nlength=`echo $1 | wc -c`; local sure=""
	set_color $SURE_FCOLOR $SURE_BCOLOR
	draw_squa $((${SCR_LENS}/2-1)) $((${SCR_COLS}/2-15)) 3 30 $bcolor
	echo -en "[1;${fcolor}m"
	##tput $((${SCR_LENS}/2+1)) $((${SCR_COLS}/2
	echo -en "[$((${SCR_LENS}/2+1));$((${SCR_COLS}/2-${nlength}/2-5))H"
	tput blink; echo -n $1; tput sgr0 
	echo -en "[1;${bcolor}m[1;${fcolor}m" 
	echo -n "(y|n)? [y] "; tput cnorm 
	set_color $old_fcolor $old_bcolor
	if read sure; then 
		case $sure in
		n* ) return 1;;
		* ) return 0;;
		esac
	fi
	return 1
}

##
## chess special global variable:
##	MAGIC_NUMS	MAN_NUMS
##	MAGIC_LENS	MAGIC_COLS
##	MAGIC_BCOLOR1	MAGIC_BCOLOR2
##	YELLOW_FCOLOR	WHITE_FCOLOR
##
init_scr_const( ) {
	MAGIC_BCOLOR1=43; MAGIC_BCOLOR2=42
	YELLOW_FCOLOR=33; WHITE_FCOLOR=37
	MAGIC_NUMS=8
	MAGIC_LENS=$((($SCR_LENS-1)/$MAGIC_NUMS))
	MAGIC_COLS=$(($SCR_COLS/$MAGIC_NUMS))
	MAN_NUMS=16
	##olde_y=1; olde_x=1; olde_fcolor=1; olde_string=""; olde_n=0;
	MAN_COLOR=33; COM_COLOR=37
}

init_scr_set( ) {
	mychild1=0; mychild2=0
	tput init
	SCR_LENS=`tput lines`; SCR_COLS=`tput cols`
	##trap "tput cup $SCR_LENS 0; stty echo; echo -en "[m"; exit 0" 0
	trap "stty echo; echo -en \"[m\"; /usr/bin/clear; \
	echo -n \"[$((${SCR_LENS}+1));1H\"; \
	if [ -p ~/action ] || [ -f ~/action ]; then rm -f ~/action; fi;\
	if [ -p ~/reply ] || [ -f ~/reply ]; then rm -f ~/reply; fi;\
	kill -s 15 $mychild1; kill -s 15 $mychild2; /usr/bin/clear; \
	stty echo; tput sgr0; tput cnorm; exit 0" 0
	##exit 0" 0
	trap "exit 2" 1 2 3 15
	fcolor=37; bcolor=40
	set_color 0 40
}

reset_scr( ) {
	echo -e "[0m"
}

##
## draw library for chess
## end of drawlib.sh
##

