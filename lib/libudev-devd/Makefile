PACKAGE=lib${LIB}

.include <src.opts.mk>

LIB=		udev
MAJOR=          0
MINOR=          43
MICRO=          2
SHLIB_MAJOR=    ${MAJOR}

SRCS=   udev-device.c \
        udev.c \
        udev-enumerate.c \
        udev-filter.c \
        udev-hwdb.c \
        udev-list.c \
        udev-monitor.c \
        udev-queue.c \
        udev-utils.c \
        utils.c 

INCS=	libudev.h \
        tree.h \
        udev-device.h \
        udev-filter.h \
        udev-list.h \
        udev-utils.h \
        udev.h \
        utils.h

.if USE_GPL
SRCS+=  utils-gpl.c
INCS+=  utils-gpl.h
.endif

CFLAGS+= -I${.CURDIR} -I${MAKEOBJDIR} -O2 -fPIC -pthread \
	-I${SRCTOP}/CoreServices/WindowServer/libevdev/include \
        -fvisibility=hidden -D_GNU_SOURCE -DHAVE_DEVINFO_H \
	-DHAVE_LIBPROCSTAT_H -DHAVE_DEV_HID_HIDRAW_H \
	-DHAVE_SYS_TREE_H -DHAVE_DEVNAME_R -DHAVE_PIPE2 \
	-DHAVE_STRCHRNUL -DHAVE_STRLCPY -DHAVE_SYSCTLBYNAME \
        -DHAVE_LINUX_INPUT_H -DTLS=__thread -DMESON_BUILD_ROOT=\"\" 
LDFLAGS+= -pthread -L${OBJTOP}/lib/libdevinfo -ldevinfo
LDFLAGS+= -L${OBJTOP}/lib/libprocstat -lprocstat

WARNS?=	2

config.h::
	touch config.h

pixman-version.h: pixman/pixman-version.h.in
	sed -e "s/@PIXMAN_VERSION_MAJOR@/${MAJOR}/" \
            -e "s/@PIXMAN_VERSION_MINOR@/${MINOR}/" \
            -e "s/@PIXMAN_VERSION_MICRO@/${MICRO}/" \
            ${.CURDIR}/pixman/pixman-version.h.in > pixman-version.h

.include <bsd.lib.mk>

${OBJS}: config.h

