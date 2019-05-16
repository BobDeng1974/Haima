#!/bin/sh
mmc_dir="/run/media/mmcblk3p1"
update_time_path="/home/chinagps/update_time.txt"


old_zImage_path="/home/chinagps/old_file/zImage"
zImage_patch_file="/home/chinagps/update/zImage.patch"
zImage_file="/home/chinagps/update/zImage"

old_dtb_path="/home/chinagps/old_file/imx6q-sabresd.dtb"
dtb_patch_file="/home/chinagps/update/imx6q-sabresd.dtb.patch"
imx6q_sabresd_file="/home/chinagps/update/imx6q-sabresd.dtb"

old_uboot_path="/home/chinagps/old_file/u-boot.imx"
uboot_patch_file="/home/chinagps/update/u-boot.imx.patch"
uboot_file="/home/chinagps/update/u-boot.imx"


old_app_path="/home/chinagps/old_file/t6a_app"
app_patch_file="/home/chinagps/update/t6a_app.patch"
app_file="/home/chinagps/update/t6a_app"
local_app_file="/home/chinagps/t6a_app"

suanfa_patch_file="/home/chinagps/update/suanfa.tar.bz2.patch"
suanfa_file="/home/chinagps/update/suanfa.tar.bz2"


if [ "$1" == "2" ];then
 echo "update zImage start"
 if [ "$2" == "1" ];then 
	bspatch "$old_zImage_path" "$zImage_file" "$zImage_patch_file"
 fi
	chmod 777 "$zImage_file"
	cp "$zImage_file" "$mmc_dir"
	sync
fi

if [ "$1" == "3" ];then
	echo "update device tree start"
 if [ "$2" == "1" ];then 
	bspatch "$old_dtb_path" "$imx6q_sabresd_file" "$dtb_patch_file"
 fi	 
  chmod 777 "$imx6q_sabresd_file"
  cp "$imx6q_sabresd_file" "$mmc_dir"
  sync
fi


if [ "$1" == "4" ];then
	echo "update uboot start"
 if [ "$2" == "1" ];then 
	bspatch "$old_uboot_path" "$uboot_file" "$uboot_patch_file"
 fi	 
 
 	echo 0 > /sys/block/mmcblk3boot0/force_ro
	dd if="$uboot_file" of=/dev/mmcblk3boot0 bs=512 seek=2
	echo 1 > /sys/block/mmcblk3boot0/force_ro
	echo 8 > /sys/block/mmcblk3/device/boot_config	
  sync
fi



if [ "$1" == "0" ];then
 echo "update app start"
 if [ "$2" == "1" ];then 
	bspatch "$old_app_path" "$app_file" "$app_patch_file"
 fi
	chmod 777 "$app_file"
	killall t6a_app
  cp "$app_file" "$local_app_file"
  sync
  echo "update app  end reboot"
fi



if [ "$1" == "1" ];then
	 echo "update arithmetic  start"
 if [ "$2" == "1" ];then 
 /home/chinagps/ota_patch.sh /home/chinagps/old_file/newpath /home/chinagps/old_file/oldpath /home/chinagps/old_file/p
 exit 0
 fi	 
 	rm /home/tsh -rf
 	cp "$suanfa_file" /xxx.tar.bz2
 	chmod 777 /xxx.tar.bz2
  tar oxvf /xxx.tar.bz2 -C /
  sync
  rm /xxx.tar.bz2
  sync
  echo "update arithmetic  end"
  #date "+%Y-%m-%d %H:%M:%S" > "$update_time_path"
	killall t6a_app
	echo "reboot"
fi

exit 0