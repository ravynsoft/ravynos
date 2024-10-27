# Common settings for building frameworks
SYSROOT=  --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
OPTIMIZE= -O0 -g
STD_DEFS= -D__RAVYNOS__ -DPLATFORM_IS_POSIX -DGCC_RUNTIME_3 \
	  -DPLATFORM_USES_BSD_SOCKETS -D__MACH__  -D__WSRPC
OBJCFLAGS= -fobjc-runtime=gnustep-2.0 -fobjc-nonfragile-abi -fPIC \
	   -Wno-missing-method-return-type -Wno-macro-redefined
FMWK_CFLAGS := ${SYSROOT} ${OPTIMIZE} ${STD_DEFS} \
	 ${OBJCFLAGS} -I${SRCTOP}/Frameworks \
	 -I${SRCTOP}/sys -I${SRCTOP}/include -I${OBJTOP}/tmp/usr/include
FMWK_CXXFLAGS := -nostdinc -nobuiltininc ${OPTIMIZE} ${STD_DEFS} \
	 ${OBJCFLAGS} -isysroot${OBJTOP}/tmp \
	 -cxx-isystem${OBJTOP}/tmp/usr/include/c++/v1 \
	 -cxx-isystem${OBJTOP}/tmp/usr/include \
	 -I${SRCTOP}/Frameworks
FMWK_LDFLAGS+= -L${BUILDROOT}/usr/lib -Wl,--no-as-needed

MK_AUTO_OBJ=    yes

.if "${NO_FMWK_COMMON}" != "yes"
beforebuild: make-obj-dirs obj ${MAKEOBJDIR}/${FRAMEWORK}.framework

.include <sys.mk>

make-obj-dirs:
	mkdir -p ${OBJTOP}/Frameworks/${FRAMEWORK}
.for subdir in ${SUBDIR}
	mkdir -p ${OBJTOP}/Frameworks/${FRAMEWORK}/${subdir}
.endfor
.endif

_fmwk_install_includes: .PHONY
	for header in ${INCS}; do \
		cp -fv ${.CURDIR}/$$header \
		${MAKEOBJDIR}/${FRAMEWORK}.framework/Headers/; \
	done

std_install_hook:
	mv ${MAKEOBJDIR}/lib${FRAMEWORK}.so \
		${MAKEOBJDIR}/${FRAMEWORK}.framework/Versions/Current/
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${MAKEOBJDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

alt_install_hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${MAKEOBJDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/


.if !target(_libinstall)
_libinstall: .PHONY
.endif

installincludes: _fmwk_install_includes
