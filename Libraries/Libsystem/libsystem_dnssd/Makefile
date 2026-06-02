MK_WERROR = no
LIB = system_dnssd
.PATH: ${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder
SLF = /System/Library/Frameworks

SRCS = \
	mDNSShared/dnssd_ipc.c \
	mDNSShared/dnssd_clientlib.c \
	mDNSMacOSX/DNSServiceDiscovery.c \
	mDNSShared/dnssd_clientstub.c

CFLAGS = -DAPPLE_OSX_mDNSResponder -D_DARWIN_C_SOURCE \
	-D__DARWIN_NON_CANCELABLE -fno-common --sysroot=${RAVYN_SDKROOT} \
	-mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wno-incompatible-sysroot -Wno-nullability-completeness \
	-F${RAVYN_SDKROOT}/System/Library/PrivateFrameworks \
	-F${RAVYN_SDKROOT}/System/Library/Frameworks \
	-I${RAVYN_SDKROOT}/usr/local/include/kernel \
	-I${RAVYN_SDKROOT}/usr/local/include \
	-I${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSShared \
	-I${ROOT_SOURCE_DIR}/BSD/usr.sbin/mDNSResponder/mDNSCore \
	-I${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch \
	-I${RAVYN_SDKROOT}${SLF}/System.framework/Versions/B/PrivateHeaders \
	-I${RAVYN_SDKROOT}${SLF}/Kernel.framework/Versions/A/Headers

LDFLAGS = -mmacos-version-min=${MACOS_VERSION_MIN} -umbrella System \
	-Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
	-L${RAVYN_SDKROOT}/usr/lib/system -L${RAVYN_SDKROOT}/usr/lib \
	-Wl,-install_name,/usr/lib/system/libsystem_dnssd.dylib \
	-Wl,-current_version,1096.100.3 -Wl,-compatibility_version,1 \
	-ldispatch -lsystem_malloc -lsystem_platform -lsystem_asl \
	-lsystem_c -lsystem_kernel -ldyld

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.dylib: lib${LIB}.a
	${CC} -fuse-ld=${LD} -shared -dylib -o ${.TARGET} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/system/

#SUBDIR = libdns_services

.include <bsd.lib.mk>
.include <bsd.subdir.mk>
