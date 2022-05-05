APP=	Trash
SRCS=	main.mm Trash.mm
RESOURCES=	${.CURDIR}/Trash.png ${.CURDIR}/Trash_full.png

Trash.o: Trash.mm Trash.h

MK_WERROR=	no
CXXFLAGS+=	-g -fPIC
LDFLAGS+=	-lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11


.include <rvn.app.mk>

all:
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
