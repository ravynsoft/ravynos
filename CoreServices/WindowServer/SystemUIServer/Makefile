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
                ${.CURDIR}/ReleaseLogo.tiff ${.CURDIR}/shutdown ${.CURDIR}/kickSession

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -D__RAVYNOS__ -D__MACH__ \
		-I${.CURDIR} -I${MAKEOBJDIR} -I${TOPDIR}/Frameworks/AppKit \
		-I${TOPDIR}/CoreServices -I${OBJTOP}/Frameworks/Foundation/Headers \
		-F${OBJTOP}/Frameworks/CoreFoundation -I${SRCTOP}/include \
		-I${OBJTOP}/tmp/usr/include \
                -framework Foundation -framework AppKit -framework LaunchServices
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_PKGS} \
		-L${OBJTOP}/Frameworks/OpenGL/OpenGL.framework/Resources \
		-Wl,-R/System/Library/Frameworks/OpenGL.framework/Versions/A \
                -lobjc -lwayland -lSystem -lm

${.CURDIR}/shutdown: shutdown.c
	${CC} -o ${.TARGET} ${.ALLSRC}

${.CURDIR}/kickSession: kickSession.m
	${CC} -O0 -g -o ${.TARGET} ${.ALLSRC} -framework AppKit -lSystem

.include <rvn.app.mk>
