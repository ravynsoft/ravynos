MK_WERROR = no

# Don't try to link System or Foundation to this
MK_OBJC_RUNTIME = no
LIB = objc.A
OBJC = ${CC} -ObjC
OBJCXX = ${CC} -ObjC++
SRCS =	runtime/hashtable2.mm \
	runtime/maptable.mm \
	runtime/objc-auto.mm \
	runtime/objc-cache.mm \
	runtime/objc-class-old.mm \
	runtime/objc-class.mm \
	runtime/objc-errors.mm \
	runtime/objc-exception.mm \
	runtime/objc-file.mm \
	runtime/objc-initialize.mm \
	runtime/objc-layout.mm \
	runtime/objc-load.mm \
	runtime/objc-loadmethod.mm \
	runtime/objc-lockdebug.mm \
	runtime/objc-runtime-new.mm \
	runtime/objc-runtime-old.mm \
	runtime/objc-runtime.mm \
	runtime/objc-sel-set.mm \
	runtime/objc-sel.mm \
	runtime/objc-sync.mm \
	runtime/objc-typeencoding.mm \
	runtime/Object.mm \
	runtime/Protocol.mm \
	runtime/OldClasses.subproj/List.m \
	runtime/Messengers.subproj/objc-msg-x86_64.s \
	runtime/objc-accessors.mm \
	runtime/objc-references.mm \
	runtime/objc-os.mm \
	runtime/objc-file-old.mm \
	runtime/objc-block-trampolines.mm \
	runtime/objc-sel-table.s \
	runtime/objc-weak.mm \
	runtime/NSObject.mm \
	runtime/objc-opt.mm \
	runtime/objc-zalloc.mm \
	runtime/objc-cache-old.mm \
	runtime/objc-sel-old.mm \
	macho.c
	# ./runtime/objc-probes.d # FIXME: need to generate .h from this

CFLAGS.objc-auto.o = -fexceptions
CFLAGS.objc-exception.o = -fexceptions
CFLAGS.objc-lockdebug.o = -Os

COMMONFLAGS = --sysroot=${RAVYN_SDKROOT} \
		-mmacos-version-min=${MACOS_VERSION_MIN} \
		-momit-leaf-frame-pointer \
		-fdollars-in-identifiers \
		-fobjc-legacy-dispatch \
		-fno-objc-convert-messages-to-runtime-calls \
		-fblocks \
		-Wno-deprecated-declarations \
		-I${.CURDIR}/runtime \
		-I${ROOT_SOURCE_DIR}/Kernel/xnu/bsd \
		-I${.OBJDIR}/../Libsystem/libsystem_kernel/mig_hdr/include \
		-I${.CURDIR}/../Libsystem/private \
		-I${.CURDIR}/../Libsystem/libsystem_blocks \
		-I${RAVYN_SDKROOT}/usr/local/include \
		-I${RAVYN_SDKROOT}/usr/local/include/kernel \
		-I${RAVYN_SDKROOT}/System/Library/Frameworks/System.framework/Versions/B/PrivateHeaders \
		-I${ROOT_BINARY_DIR}/${CpuArch} ${CFLAGS.${.TARGET:T}}

# SUPPORT_PREOPT and __OBJC2__ add support for the dyld objc shared cache
COMMONFLAGS += -D_LIBC_NO_FEATURE_VERIFICATION -DTARGET_OS_IOSMAC=0 \
	      -DTARGET_OS_OSX=1 -DNDEBUG -D__OBJC2__ -DSUPPORT_PREOPT

CFLAGS = ${COMMONFLAGS} -std=gnu11
OBJCFLAGS = ${COMMONFLAGS} -std=gnu11 \
	    -fno-objc-convert-messages-to-runtime-calls 
OBJCXXFLAGS = ${COMMONFLAGS} -std=gnu++14 \
	      -fno-objc-convert-messages-to-runtime-calls 
CXXFLAGS = ${COMMONFLAGS} -std=gnu++14 -D_LIBCPP_VISIBLE=""
ACFLAGS = ${COMMONFLAGS}

LDFLAGS = -Wl,-syslibroot,${RAVYN_SDKROOT}/usr/lib \
	-L${RAVYN_SDKROOT}/usr/lib -L${RAVYN_SDKROOT}/usr/lib/system \
	-umbrella System -mmacos-version-min=${MACOS_VERSION_MIN} \
	-Wl,-install_name,/usr/lib/libobjc.dylib -Wl,-current_version,228 \
	-Wl,-compatibility_version,1 -Wl,-allowable_client,libSystem \
	-Wl,-allowable_client,System -Wl,-allowable_client,Foundation \
	-Wl,-order_file,${.CURDIR}/libobjc.order \
	-Wl,-interposable_list,${.CURDIR}/interposable.txt \
	-Wl,-sectalign,__DATA,__data,0x1000 -lc++ -lCrashReporterClient \
	-Wl,-unexported_symbols_list,${.CURDIR}/unexported_symbols \
	-lobjc-trampolines -lsystem_c -lsystem_platform -lsystem_pthread \
	-lsystem_malloc -lunwind -lsystem_blocks -lsystem_kernel \
	-ldispatch -ldyld

all: ${.OBJDIR}/libobjc-trampolines.dylib ${.OBJDIR}/libobjc.A.dylib

${.OBJDIR}/libobjc-trampolines.dylib: ${.CURDIR}/runtime/objc-blocktramps-x86_64.s
	${CC} --target=${CpuArch}-apple-darwin -x assembler-with-cpp \
	  -mmacos-version-min=${MACOS_VERSION_MIN} -umbrella System \
	  --sysroot=${RAVYN_SDKROOT} -fpic -shared -Wl,-dylib \
	  -fuse-ld=${LD} -o ${.OBJDIR}/libobjc-trampolines.dylib \
	  -Wl,-install_name,/usr/lib/libobjc-trampolines.dylib \
	  -Wl,-current_version,228 \
	  -L${RAVYN_SDKROOT}/usr/lib/system -ldyld \
	  ${.ALLSRC}
	mkdir -p ${SYSROOT_DIR}/usr/lib ${RAVYN_SDKROOT}/usr/lib
	cp -f ${.OBJDIR}/libobjc-trampolines.dylib ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${.OBJDIR}/libobjc-trampolines.dylib ${SYSROOT_DIR}/usr/lib/

${.OBJDIR}/libobjc.A.dylib: lib${LIB}.a ${.OBJDIR}/libobjc-trampolines.dylib
	${CC} -fuse-ld=${LD} -shared -o ${.TARGET} ${OBJS} ${LDFLAGS} \
		-L${RAVYN_SDKROOT}/usr/lib -lobjc-trampolines
	mkdir -p ${RAVYN_SDKROOT}/usr/lib ${SYSROOT_DIR}/usr/lib
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${.TARGET} ${SYSROOT_DIR}/usr/lib/
	ln -sf libobjc.A.dylib ${RAVYN_SDKROOT}/usr/lib/libobjc.dylib
	ln -sf libobjc.A.dylib ${SYSROOT_DIR}/usr/lib/libobjc.dylib

#add_executable(objcdt STATIC
#	./runtime/objc-cache.mm
#	./objcdt/json.mm
#	./objcdt/objcdt.mm
#	./runtime/objc-probes.d
#)

.include <bsd.lib.mk>
