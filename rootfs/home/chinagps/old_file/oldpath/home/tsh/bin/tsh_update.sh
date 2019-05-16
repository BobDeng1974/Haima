#!/bin/sh

BOOT_DIR=/run/media/mmcblk3p1

OVERLAY_FILE=V*-*-360-overlay-*.tar.bz2

START_SH_FILE=update_start.sh
STOP_SH_FILE=update_stop.sh

# update files
#	param: $1 -- firmware dir
update_firmware() {
	for overlay in `ls $1/$OVERLAY_FILE`
	do
		if [ -f $overlay ] ; then

			echo "Updating '$UPDATE_FILE' ..."
			killall -INT switch tshmain tshcali gtxx > /dev/null 2>&1

			[ -f $1/$START_SH_FILE ] && sh $1/$START_SH_FILE > /dev/null

			cp -rvf $1/zImage $BOOT_DIR/
			cp -rvf $1/*.dtb $BOOT_DIR/
			tar xvf $1/modules.tar.bz2 -C /
			sync

			rm -rf /home/tsh/1_tianshuang/car*.tsh
			tar oxvf $overlay -C /
			sync
			sync
			sleep 1

			[ -f $1/$STOP_SH_FILE ] && sh $1/$STOP_SH_FILE > /dev/null

			cat /dev/zero > /dev/fb0
			echo "Update done!"

			return 0
		fi
	done
	return 1
}

update_test() {
	for overlay in `ls $1/tes*`
	do
		if [ -f $overlay ] ; then

			echo "Updating..."
			sleep 1			
			echo "Update done"
			return 0
		fi
	done
	return 1
}

SD_DIR=/run/media/mmcblk1p[1-9]/UPDATE
USB_DIR=/run/media/sd[a-c][1-9]/UPDATE
# SD_DIR=/home/root/aa[1-9]
# USB_DIR=/home/root/bb[1-9]

sleep 1

for dir in $SD_DIR $USB_DIR
do
	[ -d $dir ] && echo [[[ $dir ]]] && update_firmware $dir && exit 0
	# [ -d $dir ] && echo [[[ $dir ]]] && update_test $dir && exit 0
done

echo "Update error!"
