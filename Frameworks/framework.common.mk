# Common settings for building frameworks
FMWK_CFLAGS := --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
	 -O0 -g -D__RAVYNOS__ -DPLATFORM_IS_POSIX -DGCC_RUNTIME_3 \
	 -DPLATFORM_USES_BSD_SOCKETS -I/usr/include/freetype2 \
	 -I/usr/include/fontconfig -fobjc-runtime=gnustep-2.0 \
	 -fobjc-nonfragile-abi -fPIC -Wno-missing-method-return-type -Wno-macro-redefined
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

.if !target(_libinstall)
_libinstall: .PHONY
.endif

