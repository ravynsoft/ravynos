APP=	Downloads
SRCS=	main.mm Downloads.mm
RESOURCES=	${.CURDIR}/Downloads.png

Downloads.o: Downloads.mm Downloads.h

MK_WERROR=	no
CXXFLAGS+=	-g -fPIC
LDFLAGS+=	-lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <rvn.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
