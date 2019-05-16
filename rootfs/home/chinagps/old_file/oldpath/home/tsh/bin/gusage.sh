#!/bin/sh -e

IDLE_FILE=/sys/kernel/debug/gc/idle
# file format :
# ===== file start
# Start:   6372910881092 ns
# End:     6389794985760 ns
# On:      6169923008 ns
# Off:     0 ns
# Idle:    0 ns
# Suspend: 10714181660 ns
# ===== file end

if [ ! -f $IDLE_FILE ] ; then
	echo Please enable debugfs in kernel and mount it.
	exit 1
fi

TTT=`cat $IDLE_FILE`

# RND_FILE=`echo /tmp/tmp-$$$$`
RND_FILE=`echo /tmp/tmp-gpuinfo`

#echo -en "\033[2J"

while [ true ]
do
	sleep 0.5
	cat $IDLE_FILE > $RND_FILE

	TT_ON=`sed -n '/On:/p' $RND_FILE | awk '{printf "%.2f", $2}'`
	TT_SS=`sed -n '/Suspend:/p' $RND_FILE | awk '{printf "%.2f", $2}'`

	if [ "x$TT_ON" == "x0.00" ] ; then
		TT_RT="0.00"
	else
		TT_RT=`echo "scale=2;$TT_ON * 100.00 / ($TT_ON + $TT_SS)" | bc`
	fi
	
#	echo -en "\033[1;1H"
#	echo -en "\r"
	echo -en "\rGPU Usage : $TT_RT% "

done
