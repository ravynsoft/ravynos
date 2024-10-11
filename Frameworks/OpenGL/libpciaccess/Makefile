.include <src.opts.mk>

.PATH:          ${.CURDIR}
.PATH:          ${.CURDIR}/src

LIB=		pciaccess
PACKAGE=        lib${LIB}
SHLIB_MAJOR=    0

SRCS=   common_bridge.c \
        common_iterator.c \
        common_init.c \
        common_interface.c \
        common_io.c \
        common_capability.c \
        common_device_name.c \
        common_map.c \
        common_vgaarb_stub.c \
        freebsd_pci.c 

INCS=	        
CFLAGS+=        -I${.CURDIR}/include -I${.CURDIR} \
                -I${MAKEOBJDIR} \
                -DPCIIDS_PATH=\"/etc\" \
                -D_GNU_SOURCE -DHAVE_ZLIB \
                -Wno-error=implicit-function-declaration \
                -Wno-error=deprecated-declarations
LDFLAGS+=       -lz

WARNS?=	2

# Newly-built ar might need libmd and liblzma from the updated base
LD_LIBRARY_PATH+=	${OBJTOP}/tmp/lib
.export LD_LIBRARY_PATH

config.h: .PHONY
	echo >${.TARGET}

beforebuild: obj

.include <bsd.lib.mk>

${OBJS}: config.h
