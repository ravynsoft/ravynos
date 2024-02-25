PACKAGE=lib${LIB}
.PATH: ${PACKAGE}

.include <src.opts.mk>

LIB=		evdev
SHLIB_MAJOR=    2

SRCS=           prep .WAIT \
	        libevdev/libevdev-names.c \
                libevdev/libevdev-uinput.c \
                libevdev/libevdev.c \

INCSDIR=        ${INCLUDEDIR}/${PACKAGE}
INCS=	        libevdev/libevdev-uinput.h \
                libevdev/libevdev.h

CFLAGS+=        -I${.CURDIR}/libevdev \
                -I${.CURDIR}/include/linux \
                -I${SRCTOP}/sys/compat/linuxkpi/common/include \
                -I${MAKEOBJDIR} \
                -DHAVE_DLFCN_H \
                -DHAVE_INTTYPES_H \
                -DHAVE_LIBM \
                -DHAVE_MEMORY_H \
                -DHAVE_STDINT_H \
                -DHAVE_STDLIB_H \
                -DHAVE_STRINGS_H \
                -DHAVE_STRING_H \
                -DHAVE_SYS_STAT_H \
                -DHAVE_SYS_TYPES_H \
                -DHAVE_UNISTD_H \
                -DSTDC_HEADERS \
                -D_GNU_SOURCE
WARNS?=	2

prep: .PHONY
	echo > ${MAKEOBJDIR}/config.h
prep2: .PHONY
	mkdir -vp ${OBJTOP}/tmp/${INCSDIR}

.include <bsd.lib.mk>

includes: prep2
