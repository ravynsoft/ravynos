APP=	Trash
SRCS=	main.m Trash.m
RESOURCES=	${.CURDIR}/Trash.png ${.CURDIR}/Trash_full.png

Trash.o: Trash.m Trash.h

MK_WERROR=	no
CFLAGS+=	-g -fPIC -fobjc-arc
LDFLAGS+=	-framework Foundation -framework LaunchServices -lobjc


.include <rvn.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
