LIB=	        mtdev
MAJOR=          1
MINOR=          1
MICRO=          5
VERSION=        ${MAJOR}.${MINOR}.${MICRO}

.PATH:          ${.CURDIR}/src
SRCS= caps.c core.c iobuf.c match.c match_four.c
INCS=

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-I${.CURDIR}/src -I${.CURDIR}/include -I${MAKEOBJDIR} \
		-I${.CURDIR}/../libevdev/include/linux \
		-I${SRCTOP}/sys/compat/linuxkpi/common/include \
		-I${SRCTOP}/sys/compat/linuxkpi/dummy/include \
		-I${SRCTOP}/sys/compat/linux \
		-D__RAVYNOS__ -D__u8=uint8_t -D__u16=uint16_t \
		-D__u32=uint32_t -D__s8=int8_t -D__s16=int16_t \
		-D__s32=int32_t -D__u64=uint64_t -D__s64=int64_t \
		-D_IOC_NONE=1U -D_IOC_READ=2U -D_IOC_WRITE=4U -fPIC
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources'

.include <bsd.lib.mk>

/usr/obj/usr/src/amd64.amd64/CoreServices/WindowServer/libevdev/config.h:#define _IOC_NONE 1U
/usr/obj/usr/src/amd64.amd64/CoreServices/WindowServer/libevdev/config.h:#define _IOC_READ 2U
/usr/obj/usr/src/amd64.amd64/CoreServices/WindowServer/libevdev/config.h:#define _IOC_WRITE 4U
