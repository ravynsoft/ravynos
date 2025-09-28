.include <bsd.init.mk>
LIB=	osxsupport

.PATH: .
.PATH: ${.CURDIR}/uuid
.PATH: ../../sbin/launchd

BASE_INCLUDE= -I${.CURDIR}/../../include/apple
BASE_INCLUDE+= -I${.CURDIR}/../../include -I${.CURDIR}/../../sys -I${.CURDIR}
DEFINES= -D__APPLE__ -DLIBC_NO_LIBCRASHREPORTERCLIENT -DPRIVATE
MIG_FLAGS= ${BASE_INCLUDE} 	${DEFINES}
CFLAGS+= ${MIG_FLAGS}  -DHAVE_INTTYPES_H -fblocks

#support
SRCS+= assumes.c libproc.c getiopolicy_np.c fileport.c
SRCS+= si_module.c search_module.c si_data.c cache_module.c file_module.c

#uuid
SRCS+= clear.c copy.c gen_uuid.c isnull.c unparse.c pack.c unpack.c


CLEANFILES+= *~ cscope.*

.include <bsd.lib.mk>
