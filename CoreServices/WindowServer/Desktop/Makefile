APP=	        SystemUIServer
SRCS=           SystemUIServer.m \
                AppDelegate.m \
                ClockView.m \
                MenuView.m \
                ExtrasView.m \
                DesktopWindow.m \
                MenuBarWindow.m
RESOURCES=	${.CURDIR}/../Icon.png ${.CURDIR}/ravynos-mark-64.png

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} \
                -framework Foundation -framework AppKit
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_PKGS} \
                -lobjc -lwayland-client -lSystem 

.include <rvn.app.mk>

