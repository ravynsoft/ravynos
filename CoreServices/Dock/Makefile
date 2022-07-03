APP=	Dock
SRCS=	main.mm Dock.mm DockItem.mm WindowTracker.mm Utils.mm
RESOURCES=	${.CURDIR}/Dock.png ${.CURDIR}/running.png \
		${.CURDIR}/window.png Trash.app \
		Downloads.app

Dock.o: Dock.mm Dock.h Dock.moc
DockItem.o: DockItem.mm DockItem.h

MK_WERROR=	no
CXXFLAGS+=	-g -fPIC -I${MAKEOBJDIR} -fobjc-arc
LDFLAGS+=	-framework AppKit -framework LaunchServices -lSystem

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
