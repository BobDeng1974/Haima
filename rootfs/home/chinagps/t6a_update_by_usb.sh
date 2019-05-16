#!/bin/sh
sd_dir="/run/media/sda1"
mmc_dir="/run/media/mmcblk3p1"
zImage_file="/run/media/sda1/UPDATE/zImage"
imx6q_sabresd_file="/run/media/sda1/UPDATE/imx6q-sabresd.dtb"
sf_file="/run/media/sda1/UPDATE/V*-*.tar.bz2"
app_file="/run/media/sda1/UPDATE/t6a_app"
local_app_file="/home/chinagps/t6a_app"
version_path="/home/tsh/1_tianshuang/version.txt"
UPDATE_path="/run/media/sda1/UPDATE/"
update_time_path="/home/chinagps/update_time.txt"
uboot_file="/run/media/sda1/UPDATE/u-boot.imx"
msg_info="/run/media/sda1/UPDATE/msg_info.bmp"
shell_file="/run/media/sda1/UPDATE/auto_run.sh"
need_reboot=0
need_show=0
if [ -d "$sd_dir" ]
then
echo "USB Device exist"
sleep 3
else
echo "not check USB Device"
exit 0
fi

if [ -f "$shell_file" ]
then
	chmod 777 "$shell_file"
	sh "$shell_file"
	need_show=1
else
echo "not found auto_run.sh"
fi


if [ -f "$zImage_file" ]
then
 echo "update zImage start"
	chmod 777 "$zImage_file"
	cp "$zImage_file" "$mmc_dir"
	sync
	need_reboot=0
	need_show=1
else
echo "not found zImage"
fi


if [ -f "$uboot_file" ]
then
 echo "update uboot start"
	chmod 777 "$uboot_file"
	echo 0 > /sys/block/mmcblk3boot0/force_ro
	dd if="$uboot_file" of=/dev/mmcblk3boot0 bs=512 seek=2
	echo 1 > /sys/block/mmcblk3boot0/force_ro
	echo 8 > /sys/block/mmcblk3/device/boot_config	
	sync
	need_reboot=0
	need_show=1
else
echo "not found uboot"
fi



if [ -f "$imx6q_sabresd_file" ]
then
 echo "update device tree start"
        chmod 777 "$imx6q_sabresd_file"
        cp "$imx6q_sabresd_file" "$mmc_dir"
        sync
        need_reboot=0
        need_show=1
else
echo "not found dtb file"
fi

ver=$(cat ${version_path})                  
ver2="${UPDATE_path}"${ver}".tar.bz2" 
echo "$ver2"                  
ver3=$(ls /run/media/sda1/UPDATE/V*-*.tar.bz2)
echo "$ver3"
if [ -n "$ver3" ]       
then      
if [ "$ver2" = "$ver3" ]
then
	echo "sf version eq"
else
		 echo "update arithmetic  start"
		 	  rm /home/tsh -rf
        cp "$ver3" /xxx.tar.bz2
        chmod 777 /xxx.tar.bz2
        sync
        tar oxvf /xxx.tar.bz2 -C /
        sync
        rm /xxx.tar.bz2
        sync
        echo "update arithmetic  end"
        date "+%Y-%m-%d %H:%M:%S" > "$update_time_path"
	  need_reboot=1
	  need_show=1
fi
else
echo "not found arithmetic file"
fi


if [ -f "$app_file" ]
then
str1=$(strings "$app_file" | grep T6A-APP-VER-)
echo "str1 is "$str1""
str2=$(strings "$local_app_file" | grep T6A-APP-VER-)
echo "str2 is "$str2""
if [ -z "$str1" ]
then
echo "str1 is null"
fi
if [ -z "$str2" ]
then
echo "str2 is null"
fi
if [ "$str1" = "$str2" ]
then
	echo "app version eq"
else
	 echo "update app  start"
        chmod 777 "$app_file"
        date "+%Y-%m-%d %H:%M:%S" > "$update_time_path"
        killall t6a_app
        cp "$app_file" "$local_app_file"
        sync
        echo "update app  end reboot"
        need_reboot=1
        need_show=1
fi
else
echo "not found app file"
fi

if [ $need_reboot -eq 1 ]
then
	killall t6a_app
	echo "wait reboot"
fi	

if [ $need_show -eq 1 ]
then
	killall -INT switch tshmain
	sleep 3
	imgshow "$msg_info"
fi	
exit 0
