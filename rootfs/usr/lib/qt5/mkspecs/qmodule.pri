CONFIG +=  cross_compile compile_examples silent qpa largefile neon pcre
QT_BUILD_PARTS +=  examples libs tools
QT_NO_DEFINES =  ALSA CUPS EGL_X11 FONTCONFIG GLIB HARFBUZZ ICONV IMAGEFORMAT_JPEG LIBPROXY MITSHM NIS OPENVG SESSIONMANAGER SHAPE STYLE_GTK TABLET TSLIB XCURSOR XFIXES XINERAMA XINPUT XKB XRANDR XRENDER XSYNC XVIDEO ZLIB
QT_QCONFIG_PATH = 
host_build {
    QT_CPU_FEATURES.arm =  neon
} else {
    QT_CPU_FEATURES.arm =  neon
}
QT_COORD_TYPE = double
QT_LFLAGS_ODBC   = -lodbc
OPENSSL_LIBS = -lssl -lcrypto
OE_QMAKE_AR = arm-poky-linux-gnueabi-ar
OE_QMAKE_CC = arm-poky-linux-gnueabi-gcc  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo
OE_QMAKE_CFLAGS =  -O2 -pipe -g -feliminate-unused-debug-types
OE_QMAKE_COMPILER = arm-poky-linux-gnueabi-gcc  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo
OE_QMAKE_CXX = arm-poky-linux-gnueabi-g++  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo
OE_QMAKE_CXXFLAGS =  -O2 -pipe -g -feliminate-unused-debug-types -fvisibility-inlines-hidden
OE_QMAKE_LDFLAGS = -Wl,-O1 -Wl,--hash-style=gnu -Wl,--as-needed
OE_QMAKE_LINK = arm-poky-linux-gnueabi-g++  -march=armv7-a -mfloat-abi=hard -mfpu=neon -mtune=cortex-a9 --sysroot=/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo
OE_QMAKE_STRIP = echo
styles += mac fusion windows
DEFINES += QT_NO_MTDEV
QT_CFLAGS_PULSEAUDIO = -D_REENTRANT -I/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo/usr/include/glib-2.0 -I/home/bamboo/build/3.14.52-1.1.0/fsl-imx-fb/temp_build_dir/build_fsl-imx-fb/tmp/sysroots/imx6qdlsolo/usr/lib/glib-2.0/include 
QT_LIBS_PULSEAUDIO = -lpulse-mainloop-glib -lpulse -lglib-2.0 
QMAKE_INCDIR_OPENGL_ES2 = 
QMAKE_LIBDIR_OPENGL_ES2 = 
QMAKE_LIBS_OPENGL_ES2 =  "-lGLESv2" "-lEGL"
QMAKE_CFLAGS_OPENGL_ES2 = 
QMAKE_INCDIR_LIBUDEV = 
QMAKE_LIBS_LIBUDEV = -ludev -lrt 
DEFINES += QT_NO_TSLIB
DEFINES += QT_NO_LIBINPUT
QMAKE_INCDIR_EGL = 
QMAKE_LIBS_EGL = -lEGL 
QMAKE_CFLAGS_EGL = -DLINUX -DEGL_API_FB 
sql-drivers = 
sql-plugins = 
