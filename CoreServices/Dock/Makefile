APP=	Dock
SRCS=	main.mm Dock.mm DockItem.mm EventFilter.mm
RESOURCES=	Dock.png running.png Trash.app

Dock.o: Dock.mm Dock.h Dock.moc
DockItem.o: DockItem.mm DockItem.h
Dock.moc: Dock.h
	moc -o ${.TARGET} ${.ALLSRC}

QT_CFLAGS!=	pkg-config --cflags Qt5Widgets Qt5Xdg
QT_LDFLAGS!=	pkg-config --libs Qt5Widgets Qt5Xdg
CXXFLAGS+=	${QT_CFLAGS} -g -fPIC
LDFLAGS+=	${QT_LDFLAGS} -lc++ -framework Foundation \
		-framework LaunchServices -lobjc -lX11

Trash.app::
	rm -rf ${.TARGET}
	${MAKE} -C Trash
	mv Trash/${.TARGET} .

.include <airyx.app.mk>

all: Trash.app
	cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents/
