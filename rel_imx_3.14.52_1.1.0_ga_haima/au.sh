rm ../modules* -rf
./kbuildImage.sh
./makemodules.sh
./kbuild.sh
tar jcvf ../modules.tar.bz2 ../modules/
cp ../modules.tar.bz2 /home/VideoStorage/liuh/work/output_haima/
cp arch/arm/boot/zImage /home/VideoStorage/liuh/work/output_haima
cp arch/arm/boot/dts/imx6q-sabresd.dtb /home/VideoStorage/liuh/work/output_haima
cp .config /home/VideoStorage/liuh/work/output_haima
cp Module.symvers /home/VideoStorage/liuh/work/output_haima/
cp ../bootloader_haima/u-boot.imx /home/VideoStorage/liuh/work/output_haima
