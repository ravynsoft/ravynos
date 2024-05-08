APP=	        LoginWindow
SRCS=           LoginWindow.m \
                AppDelegate.m \
                DesktopWindow.m \
                LoginBox.m \
                Label.m 
RESOURCES=      ${.CURDIR}/splash-1.png

MK_WERROR=	no
WARNS=	        1
OBJCFLAGS= -fobjc-runtime=gnustep-2.0 -fobjc-nonfragile-abi -fPIC \
	   -Wno-missing-method-return-type -Wno-macro-redefined
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} \
		--sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin \
		${OBJCFLAGS} -I${SRCTOP}/Frameworks -D__MACH__ \
		-I${MAKEOBJDIR}/../SystemUIServer -D__RAVYNOS__ \
		-I${SRCTOP}/Frameworks/AppKit/Headers \
		-I${OBJTOP}/Frameworks/Foundation/Headers
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' \
		-L${OBJTOP}/Frameworks/Foundation/Foundation.framework \
		-L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework/Resources \
		-Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/OpenGL.framework/Resources \
                -lAppKit -lFoundation -lobjc -lwayland -lSystem -lcrypt -lm

.include <rvn.app.mk>
${OBJS}: obj
