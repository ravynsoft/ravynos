APP=	Downloads
SRCS=	main.m Downloads.m
RESOURCES=	${.CURDIR}/Downloads.png

Downloads.o: Downloads.m Downloads.h

MK_WERROR=	no
CFLAGS+=	-g -fPIC -fobjc-arc
LDFLAGS+=	-framework Foundation -framework LaunchServices -lobjc


.include <rvn.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
