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
                ${.CURDIR}/ReleaseLogo.tiff ${.CURDIR}/shutdown

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} -I${TOPDIR}/Frameworks/AppKit \
                -framework Foundation -framework AppKit -framework LaunchServices
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_PKGS} \
                -lobjc -lwayland-client -lSystem -lm

${.CURDIR}/shutdown: shutdown.c
	${CC} -o ${.TARGET} ${.ALLSRC}

.include <rvn.app.mk>
