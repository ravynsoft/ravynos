.include <internal/framework.common.mk>

FRAMEWORK=LaunchServices
SRCS= LaunchServices.mm LSAppRecord.mm UTTypes.mm
INCS= LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql
QT5CFLAGS!= pkg-config --cflags Qt5Xdg
QT5LDFLAGS!= pkg-config --libs Qt5Xdg
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers ${QT5CFLAGS}
LDFLAGS+= -L/usr/local/lib -lsqlite3 ${QT5LDFLAGS}

build: all open installopen

open: open.m
	clang ${CFLAGS} -g -o ${.TARGET} ${.ALLSRC} \
		-F${BUILDROOT}/System/Library/Frameworks \
		-framework LaunchServices -framework Foundation

installopen: open
	mkdir -p ${BUILDROOT}/usr/bin
	${SUDO} install -m755 -o root -g wheel open ${BUILDROOT}/usr/bin

.include <airyx.framework.mk>
