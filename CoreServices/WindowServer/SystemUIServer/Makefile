APP=	        SystemUIServer
SRCS=           SystemUIServer.m \
                AppDelegate.m \
                ClockView.m \
                MenuView.m \
                ExtrasView.m \
                MenuBarWindow.m \
                Label.m \
                AboutWindow.m
RESOURCES=	${.CURDIR}/../Icon.png ${.CURDIR}/ravynos-mark-64.png \
                ${.CURDIR}/../SneakySnek.png ${.CURDIR}/shutdown ${.CURDIR}/kickSession

MK_WERROR=	no
WARNS=	        1
OBJCFLAGS= -fobjc-runtime=gnustep-2.0 -fobjc-nonfragile-abi -fPIC \
	   -Wno-missing-method-return-type -Wno-macro-redefined
CFLAGS+=	-g -fobjc-arc ${OBJCFLAGS} -D__RAVYNOS__ -D__MACH__ \
		--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
		-B${OBJTOP}/tmp/legacy/bin \
		-I${.CURDIR} -I${MAKEOBJDIR} -I${SRCTOP}/include \
		-I${SRCTOP}/Frameworks/AppKit/Headers -I${.CURDIR}/../.. \
		-I${TOPDIR}/CoreServices -I${OBJTOP}/Frameworks/Foundation/Headers \
		-I${SRCTOP}/Frameworks -I${OBJTOP}/tmp/usr/include \
		-I${OBJTOP}/Frameworks/CoreServices/CoreServices.framework/Headers \
		-I${OBJTOP}/Frameworks/CFNetwork/CFNetwork.framework/Headers \
		-I${OBJTOP}/Frameworks/OpenGL/OpenGL.framework/Headers \
		-I${OBJTOP}/Frameworks/ApplicationServices/ApplicationServices.framework/Headers \
		-I${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework/Headers \
		-I${OBJTOP}/Frameworks/CoreText/CoreText.framework/Headers \
		-I${OBJTOP}/Frameworks/QuartzCore/QuartzCore.framework/Headers \
		-I${OBJTOP}/Frameworks/AppKit \
		-I${OBJTOP}/Frameworks/Onyx2D/Onyx2D.framework/Headers 
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' \
		-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/Frameworks/LaunchServices/LaunchServices.framework \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/LaunchServices.framework/Versions/A \
                -lAppKit -lCoreGraphics -lLaunchServices -lFoundation -lobjc -lSystem -lm

# Need this to find gl.h before toolchain supports -framework
${MAKEOBJDIR}/OpenGL:
	ln -sf ${SRCTOP}/Frameworks/OpenGL/include ${.TARGET}

${.CURDIR}/shutdown: shutdown.c
	${CC} -o ${.TARGET} ${.ALLSRC}

${.CURDIR}/kickSession: kickSession.m
	${CC} -D__RAVYNOS__ -D__MACH__ -O0 -g -o ${.TARGET} ${.ALLSRC} \
		--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
		${OBJCFLAGS} -I${SRCTOP}/Frameworks -I${MAKEOBJDIR} \
		-F${OBJTOP}/Frameworks/Foundation \
		-I${SRCTOP}/Frameworks/AppKit/Headers \
		-I${OBJTOP}/Frameworks/Foundation/Headers \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-lAppKit -lFoundation -lSystem -lobjc

.include <rvn.app.mk>

${OBJS}: ${MAKEOBJDIR}/OpenGL
