MK_WERROR = no
LIB = kmod
SRCS = c_start.c c_stop.c pure.c

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${ROOT_SOURCE_DIR}/Frameworks

LIBDIR = ${RAVYN_SDKROOT}/usr/local/lib/kernel

all: copyfile

copyfile: lib${LIB}.a
	mkdir -p ${LIBDIR}
	cp -f ${.OBJDIR}/lib${LIB}.a ${LIBDIR}

.include <bsd.lib.mk>
