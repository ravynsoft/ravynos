.include <internal/framework.common.mk>

FRAMEWORK=LaunchServices
SRCS= LaunchServices.mm LSAppRecord.mm
INCS= LaunchServices.h
RESOURCES= Info.plist
QT5CFLAGS!= pkg-config --cflags Qt5Xdg
QT5LDFLAGS!= pkg-config --libs Qt5Xdg
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers ${QT5CFLAGS}
LDFLAGS+= -L/usr/local/lib -lsqlite3 ${QT5LDFLAGS}

build: all

.include <airyx.framework.mk>
