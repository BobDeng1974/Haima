#configuration
CONFIG +=  cross_compile shared qpa no_mocdepend release qt_no_framework
host_build {
    QT_ARCH = arm
    QT_TARGET_ARCH = arm
} else {
    QT_ARCH = arm
    QMAKE_DEFAULT_LIBDIRS = $$[QT_SYSROOT]/usr/lib $$[QT_HOST_PREFIX]/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.9.2 $$[QT_SYSROOT]/lib $$[QT_SYSROOT]/usr/lib/arm-poky-linux-gnueabi/4.9.2
    QMAKE_DEFAULT_INCDIRS = $$[QT_SYSROOT]/usr/include/c++/4.9.2 $$[QT_SYSROOT]/usr/include/c++/4.9.2/arm-poky-linux-gnueabi $$[QT_SYSROOT]/usr/include/c++/4.9.2/backward $$[QT_HOST_PREFIX]/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.9.2/include $$[QT_SYSROOT]/usr/lib/gcc/arm-poky-linux-gnueabi/4.9.2/include $$[QT_HOST_PREFIX]/lib/arm-poky-linux-gnueabi/gcc/arm-poky-linux-gnueabi/4.9.2/include-fixed $$[QT_SYSROOT]/usr/include
}
QT_CONFIG +=  minimal-config small-config medium-config large-config full-config libudev evdev c++11 accessibility egl eglfs eglfs_viv opengl opengles2 shared qpa reduce_exports clock-gettime clock-monotonic posix_fallocate mremap getaddrinfo ipv6ifname getifaddrs inotify eventfd system-jpeg system-png png freetype no-harfbuzz system-zlib dbus openssl-linked pulseaudio gstreamer-1.0 concurrent audio-backend release

#versioning
QT_VERSION = 5.5.1
QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 5
QT_PATCH_VERSION = 1

#namespaces
QT_LIBINFIX = 
QT_NAMESPACE = 

QT_EDITION = OpenSource

# pkgconfig
PKG_CONFIG_SYSROOT_DIR = $$[QT_SYSROOT]
PKG_CONFIG_LIBDIR = $$[QT_SYSROOT]/usr/lib/pkgconfig

QT_GCC_MAJOR_VERSION = 4
QT_GCC_MINOR_VERSION = 9
QT_GCC_PATCH_VERSION = 2
