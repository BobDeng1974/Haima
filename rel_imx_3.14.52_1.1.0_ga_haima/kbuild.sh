#make modules_install INSTALL_MOD_PATH=../modules  ARCH=arm CROSS_COMPILE=/home/work/zhangd/gcc/arm-eabi-4.6/bin/arm-eabi- -j20
#make modules_install INSTALL_MOD_PATH=../modules ARCH=arm CROSS_COMPILE=/home/VideoStorage/liuh/cross_compile/gcc/arm-eabi-4.6/bin/arm-eabi- -j20
make modules_install INSTALL_MOD_PATH=../modules ARCH=arm CROSS_COMPILE=../tools/gcc/arm-eabi-4.6/bin/arm-eabi- -j20
