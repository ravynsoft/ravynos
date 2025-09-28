all: open

open: open.m
	${CC} ${CFLAGS} -o ${.TARGET} ${.ALLSRC} \
	--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
	-D__RAVYNOS__ -D__MACH__ -fobjc-runtime=gnustep-2.0 \
	-fobjc-nonfragile-abi -fPIC \
	-Wno-missing-method-return-type -Wno-macro-redefined \
	-F${OBJTOP}/Frameworks/Foundation \
	-I${OBJTOP}/Frameworks/Foundation/Headers \
	-I${SRCTOP}/Frameworks/LaunchServices \
	-I${SRCTOP}/Frameworks \
	-L${OBJTOP}/Frameworks/LaunchServices/LaunchServices.framework \
	-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
	-lLaunchServices -lFoundation -lobjc \
	-Wl,-R/System/Library/Frameworks/LaunchServices.framework \
	-Wl,-R/System/Library/Frameworks/Foundation.framework
	mkdir -p ${BUILDROOT}/usr/bin
	${SUDO} install -m755 -o root -g wheel open ${BUILDROOT}/usr/bin

clean:
	rm -f open

