APP=	Trash
SRCS=	main.mm Trash.mm
RESOURCES=	Trash.png

Trash.o: Trash.mm Trash.h

CXXFLAGS+=	-g -fPIC
LDFLAGS+=	-lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <airyx.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
