APP=	        LoginWindow
SRCS=           LoginWindow.m \
                AppDelegate.m \
                DesktopWindow.m \
                LoginBox.m \
                Label.m 
RESOURCES=      ${.CURDIR}/splash-1.png

MK_WERROR=	no
WARNS=	        1
CFLAGS+=	-g -fobjc-arc -I${.CURDIR} -I${MAKEOBJDIR} -I${TOPDIR}/Frameworks/AppKit \
                -framework Foundation -framework AppKit
LDFLAGS+=	-Wl,-R'$$ORIGIN/../Resources' ${LDFLAGS_PKGS} \
                -lobjc -lwayland-client -lSystem -lcrypt -lm

.include <rvn.app.mk>
