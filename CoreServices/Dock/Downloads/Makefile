APP=	Downloads
SRCS=	main.mm Downloads.mm
RESOURCES=	Downloads.png

Downloads.o: Downloads.mm Downloads.h

CXXFLAGS+=	-g -fPIC
LDFLAGS+=	-lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <airyx.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
