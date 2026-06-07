MK_WERROR = no
LIB = system_coreservices
SRCS = sysdir.c

SLF = /System/Library/Frameworks
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-nullability-completeness
TEST_CFLAGS = -g -DTEST
LDFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-current_version,114 -Wl,-compatibility_version,1 \
	-umbrella System \
	-Wl,-install_name,/usr/lib/system/libsystem_coreservices.dylib

INCS = sysdir.h
INCSDIR = ${RAVYN_SDKROOT}/usr/include

all: ${.OBJDIR}/lib${LIB}.dylib

test: sysdir.c sysdir.h
	${HOST_CC} ${TEST_CFLAGS} -o sysdir_test ${.CURDIR}/sysdir.c
	${.OBJDIR}/sysdir_test

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a copyincs
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

copyincs: ${INCS}
	mkdir -p ${INCSDIR}
.for inc in ${INCS}
	cp -f ${.CURDIR}/${inc} ${INCSDIR}/
.endfor

.include <bsd.lib.mk>

