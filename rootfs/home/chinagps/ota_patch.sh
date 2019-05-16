#!/bin/bash

BSPATCH=bspatch

function patchfunc()
{
    for x in $(ls -a1 $3)
    do
        if [ "$x" == "." ] ; then
            ls > /dev/null
        elif [ "$x" == ".." ] ; then
            ls > /dev/null
        elif [ -d $3/$x ] ; then
        	install -d $1/$x
            echo ">>>>>>>>working in $3/$x<<<<<<<<"
            patchfunc $1/$x $2/$x $3/$x
        else
        	#echo $3/$x
            file=${x:0:-2}
            echo  $x
            rm -f $1/$file
        	$BSPATCH $2/$file $1/$file $3/$x
            chmod --reference=$2/$file $1/$file
        fi
    done
}

if [ -z $3 ] ; then
    echo $0 newpath oldpath patchpath
else
		rm $1/* -rf	
		rm $3/* -rf		
    cp -r $2/* $1
    tar oxvf /home/chinagps/update/suanfa.tar.bz2.patch -C $3
    sync

    patchfunc $1 $2 $3/patch
    cp -r $3/add/* $1/
    cp -rP $3/link/* $1/
    sync
    killall -INT switch tshmain
    rm /home/tsh -rf
    cp $1/* / -rfdv
	  sync
	  echo "update arithmetic  end"
	  #date "+%Y-%m-%d %H:%M:%S" > "$update_time_path"
		killall t6a_app
		echo "reboot"
fi
