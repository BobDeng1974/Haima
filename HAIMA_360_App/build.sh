#!/bin/bash

if [ "$1" == "imx6" ]; then
#source /home/zhangd/gcc/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi
source ../tools/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi
make clean
make
fi
if [ "$1" == "t3" ]; then
make -f Makefile_T3 clean
make -f Makefile_T3
fi
