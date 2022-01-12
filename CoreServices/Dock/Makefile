APP=	Dock
SRCS=	main.mm Dock.mm DockItem.mm WindowTracker.mm Utils.mm
RESOURCES=	Dock.png running.png window.png Trash.app folder.png \
		Downloads.app

Dock.o: Dock.mm Dock.h Dock.moc
DockItem.o: DockItem.mm DockItem.h
Dock.moc: Dock.h
	moc -o ${.TARGET} ${.ALLSRC}

QT_CFLAGS!=	pkg-config --cflags Qt5Widgets Qt5Xdg
QT_LDFLAGS!=	pkg-config --libs Qt5Widgets Qt5Xdg
CXXFLAGS+=	${QT_CFLAGS} -g -fPIC -I/usr/include/KF5/KWindowSystem
LDFLAGS+=	${QT_LDFLAGS} -lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lKF5WindowSystem -lX11

Trash.app::
	rm -rf ${.TARGET}
	${MAKE} -C Trash
	mv Trash/${.TARGET} .

Downloads.app::
	rm -rf ${.TARGET}
	${MAKE} -C Downloads
	mv Downloads/${.TARGET} .

.include <airyx.app.mk>

all: Trash.app Downloads.app
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
