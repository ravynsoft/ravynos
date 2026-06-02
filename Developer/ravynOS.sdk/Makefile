.include <bsd.own.mk>

PLATFORM_DIR = ${ROOT_BINARY_DIR}/Developer/Platforms/ravynOS.platform
FPATH = /System/Library/Frameworks/System.framework
libc_path = ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_c
objc_path = ${ROOT_SOURCE_DIR}/Libraries/objc4

all: ${PLATFORM_DIR} ${TOOLS}/unifdef
	mkdir -p ${RAVYN_SDKROOT}${FPATH}/Versions/B
	mkdir -p ${RAVYN_SDKROOT}/usr/local/include
	mkdir -p ${RAVYN_SDKROOT}/usr/local/lib
	mkdir -p ${RAVYN_SDKROOT}/usr/local/bin
	mkdir -p ${RAVYN_SDKROOT}/usr/include
	mkdir -p ${RAVYN_SDKROOT}/usr/lib
	mkdir -p ${RAVYN_SDKROOT}/usr/bin
	cp -f ${.CURDIR}/SDKSettings.plist ${.CURDIR}/SDKSettings.json \
		${.CURDIR}/Entitlements.plist ${RAVYN_SDKROOT}/
	ln -sf Versions/Current/PrivateHeaders ${RAVYN_SDKROOT}${FPATH}/PrivateHeaders
	ln -sf Versions/Current/Resources ${RAVYN_SDKROOT}${FPATH}/Resources
	ln -sf B ${RAVYN_SDKROOT}${FPATH}/Versions/Current
	ln -sf Versions/Current/System ${RAVYN_SDKROOT}${FPATH}/System
	ln -sf ../../../../../../usr/lib/libSystem.B.dylib ${RAVYN_SDKROOT}${FPATH}/Versions/B/System
	ln -sf libSystem.B.dylib ${RAVYN_SDKROOT}/usr/lib/libSystem.dylib
	mkdir -p ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Versions/A
	ln -sf Versions/A/Headers ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/Headers
	ln -sf Versions/A/PrivateHeaders ${RAVYN_SDKROOT}/System/Library/Frameworks/Kernel.framework/PrivateHeaders
	# Compatibility symlinks
	ln -sf ravynOS.platform ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform
	ln -sf ravynOS.sdk ${ROOT_BINARY_DIR}/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
	cp -pRf ${.CURDIR}/usr ${.CURDIR}/System ${RAVYN_SDKROOT}/
	cp -f ${ROOT_SOURCE_DIR}/Libraries/Libsystem/private/pthread/tsd_private.h \
		${RAVYN_SDKROOT}/usr/local/include/pthread/
	${ROOT_SOURCE_DIR}/Kernel/xnu/bsd/sys/make_symbol_aliasing.sh \
		${RAVYN_SDKROOT} ${RAVYN_SDKROOT}/usr/include/sys/_symbol_aliasing.h
	env SRCROOT=${libc_path} DSTROOT=${RAVYN_SDKROOT} \
		SDK_INSTALL_HEADERS_ROOT=. "PATH=${TOOLS}:${PATH}" ARCHS="${CpuArch} " \
		VARIANT_PLATFORM_NAME=macosx bash ${libc_path}/xcodescripts/headers.sh
	cd ${ROOT_SOURCE_DIR}/Libraries/Libsystem/libsystem_pthread && \
		env DSTROOT=${RAVYN_SDKROOT} SDK_INSTALL_HEADERS_ROOT=. \
		sh xcodescripts/install-sys-headers.sh
	mkdir -p ${RAVYN_SDKROOT}/usr/include/objc
	cp -fR ${objc_path}/runtime/runtime.h ${objc_path}/runtime/objc.h \
		${objc_path}/runtime/objc-sync.h ${objc_path}/runtime/objc-exception.h \
		${objc_path}/runtime/objc-auto.h ${objc_path}/runtime/objc-api.h \
		${objc_path}/runtime/message.h ${objc_path}/runtime/NSObject.h \
		${objc_path}/runtime/NSObjCRuntime.h ${objc_path}/runtime/Module \
		${RAVYN_SDKROOT}/usr/include/objc/
	mkdir -p ${RAVYN_SDKROOT}/usr/local/include/objc
	cp -f ${objc_path}/runtime/maptable.h ${objc_path}/runtime/NSObject-internal.h \
		${objc_path}/runtime/objc-abi.h ${objc_path}/runtime/objc-block-trampolines.h \
		${objc_path}/runtime/objc-gdb.h ${objc_path}/runtime/objc-internal.h \
		${objc_path}/runtime/objc-ptrauth.h ${objc_path}/runtime/hashtable.h \
		${objc_path}/runtime/hashtable2.h ${objc_path}/runtime/objc-class.h \
		${RAVYN_SDKROOT}/usr/local/include/objc/

SUBDIR = rpcsvc

.include <bsd.subdir.mk>

