
PROG = sw_vers
SRCS = sw_vers.c
OBJS = sw_vers.o

MK_WERROR = no

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-nullability-completeness -I${ROOT_SOURCE_DIR}/Frameworks \
	-I${ROOT_SOURCE_DIR}/Kernel/xnu/libkern

LDFLAGS = -lCoreFoundation -Wl,-platform_version -Wl,macos \
	  -Wl,${MACOS_VERSION_MIN} -Wl,${MACOS_VERSION_MIN} \
	  -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib

all: ${PROG}

${.OBJDIR}/${PROG}:
	mkdir -p ${.TARGET:H}

${PROG}: ${.OBJDIR}/${PROG}

install: ${PROG}
	mkdir -p ${SYSROOT_DIR}/usr/bin
	cp -f ${.OBJDIR}/${PROG} ${SYSROOT_DIR}/usr/bin/

.include <bsd.prog.mk>
