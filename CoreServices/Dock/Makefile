APP=	Dock
SRCS=	main.m Dock.m DockItem.m DesktopWindow.m DockTileData.m
RESOURCES=	${.CURDIR}/Dock.png ${.CURDIR}/running.png \
		${.CURDIR}/window.png Trash.app \
		Downloads.app

Dock.o: Dock.m Dock.h 
DockItem.o: DockItem.m DockItem.h

MK_WERROR=	no
CFLAGS+=	-g -fobjc-arc
LDFLAGS+=	-framework LaunchServices -framework AppKit \
                -framework Foundation -lobjc -lSystem

Trash.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Trash
	mv Trash/Trash.app .

Downloads.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Downloads
	mv Downloads/Downloads.app .

.include <rvn.app.mk>

all: Trash.app Downloads.app
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
