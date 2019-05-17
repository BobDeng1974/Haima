#!/bin/bash
set -e

TOP_DIR=`pwd`

OUT=${TOP_DIR}/out/

UBOOT_DIR=${TOP_DIR}/bootloader_haima/

KER_DIR=${TOP_DIR}/rel_imx_3.14.52_1.1.0_ga_haima/

function mk_uboot(){
    echo -e "\033[47;30mUboot building ....... $*\033[0m"

    cd ${UBOOT_DIR}
    ./kbuildImage.sh

    echo -e "\033[47;30mUboot build is ok !!! $*\033[0m"
}

function mk_kernel(){
    echo -e "\033[47;30mKernel building ....... $*\033[0m"

    cd ${KER_DIR}
    ./kbuildImage.sh
    ./makemodules.sh
    ./kbuild.sh
    tar jcvf ../modules.tar.bz2 ../modules/

    echo -e "\033[47;30mKernel build is ok !!! $*\033[0m"
}


function mk_pack(){
    echo -e "\033[47;30mPacking ....... $*\033[0m"

    mv ${TOP_DIR}/modules.tar.bz2 ${OUT}
    cp ${KER_DIR}/arch/arm/boot/zImage ${OUT} 
    cp ${KER_DIR}/arch/arm/boot/dts/imx6q-sabresd.dtb ${OUT}
    cp ${KER_DIR}/.config ${OUT}
    cp ${KER_DIR}/Module.symvers ${OUT}
    cp ${UBOOT_DIR}/u-boot.imx ${OUT}

    echo -e "\033[47;30mPack is ok !!! $*\033[0m"
}

if [ -d ${OUT} ];then
    rm ${OUT} -rf
fi

mkdir -p ${OUT}

mk_uboot

mk_kernel

mk_pack

