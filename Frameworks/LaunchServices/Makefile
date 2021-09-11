.include <internal/framework.common.mk>

FRAMEWORK=LaunchServices
SRCS= LaunchServices.mm LSAppRecord.mm UTTypes.mm
INCS= LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql
QT5CFLAGS!= pkg-config --cflags Qt5Xdg Qt5DBus
QT5LDFLAGS!= pkg-config --libs Qt5Xdg Qt5DBus
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers ${QT5CFLAGS}
LDFLAGS+= -L/usr/local/lib -lsqlite3 ${QT5LDFLAGS}

build: all

.include <airyx.framework.mk>
