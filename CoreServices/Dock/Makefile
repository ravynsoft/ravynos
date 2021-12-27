APP=	Dock
SRCS=	main.mm Dock.mm

QT_CFLAGS!=	pkg-config --cflags Qt5Widgets
QT_LDFLAGS!=	pkg-config --libs Qt5Widgets
CXXFLAGS+=	${QT_CFLAGS} -g -fPIC
LDFLAGS+=	${QT_LDFLAGS} -lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <airyx.app.mk>

