LIB = copyfile
SRCS = copyfile.c xattr_flags.c


CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -D"VERSION_INFO_PREFIX=__attribute__((visibility(\"hidden\")))" \
        -DNO_QUARANTINE # until quarantine.h implemented
CFLAGS += -I${.CURDIR}/../liblaunch -I${RAVYN_SDKROOT}/usr/include \
        -I${RAVYN_SDKROOT}/usr/local/include/kernel \
        -I${.CURDIR}/../libsystem_info/membership.subproj

# same version as removefile
LDFLAGS = -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib \
        -L${RAVYN_SDKROOT}/usr/lib/system -Wl,-current_version,48 \
        -Wl,-install_name,/usr/lib/system/libcopyfile.dylib -umbrella System \
        -Wl,-compatibility_version,1 -mmacos-version-min=${MACOS_VERSION_MIN} \
        -lsystem_info -lsystem_asl -lsystem_kernel -lsystem_c -lsystem_malloc \
        -ldispatch -lsystem_blocks -lxpc -ldyld -fuse-ld=${LD}

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -shared -Wl,-dylib -o ${.TARGET} ${OBJS} ${LDFLAGS}
	mkdir -p ${SYSROOT_DIR}/usr/lib/system ${RAVYN_SDKROOT}/usr/lib/system
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/
	cd ${.CURDIR} && env PLATFORM=MacOSX DSTROOT=${RAVYN_SDKROOT} \
          sh ${.CURDIR}/xcodescripts/install_files.sh

.include <bsd.lib.mk>
