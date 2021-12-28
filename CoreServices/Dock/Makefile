APP=	Dock
SRCS=	main.mm Dock.mm
RESOURCES=	Dock.png

Dock.o: Dock.mm Dock.h

QT_CFLAGS!=	pkg-config --cflags Qt5Widgets
QT_LDFLAGS!=	pkg-config --libs Qt5Widgets
CXXFLAGS+=	${QT_CFLAGS} -g -fPIC
LDFLAGS+=	${QT_LDFLAGS} -lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <airyx.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
