APP=	Dock
SRCS=	main.mm Dock.mm DockItem.mm WindowTracker.mm Utils.mm
RESOURCES=	${.CURDIR}/Dock.png ${.CURDIR}/running.png \
		${.CURDIR}/window.png Trash.app \
		Downloads.app
MOC=	/usr/lib/qt5/bin/moc

Dock.o: Dock.mm Dock.h Dock.moc
DockItem.o: DockItem.mm DockItem.h
Dock.moc: ${.CURDIR}/Dock.h
	${MOC} -o ${.TARGET} ${.ALLSRC}

MK_WERROR=	no
QT_CFLAGS!=	pkg-config --cflags Qt5Widgets Qt5Xdg
QT_LDFLAGS!=	pkg-config --libs Qt5Widgets Qt5Xdg
CXXFLAGS+=	${QT_CFLAGS} -g -fPIC -I/usr/include/KF5/KWindowSystem \
		-I${MAKEOBJDIR} -fobjc-arc
LDFLAGS+=	${QT_LDFLAGS} -lc++ -framework Foundation -framework CoreFoundation \
		-framework LaunchServices -lobjc -lKF5WindowSystem -lX11

Trash.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Trash
	mv Trash/Trash.app .

Downloads.app::
	rm -rf ${.TARGET}
	${MAKE} -C ${.CURDIR}/Downloads
	mv Downloads/Downloads.app .

.include <airyx.app.mk>

all: Trash.app Downloads.app
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
