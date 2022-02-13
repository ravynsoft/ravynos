.include "../framework.common.mk"

FRAMEWORK=LaunchServices
SRCS= LaunchServices.mm LSAppRecord.mm UTTypes.mm LSIPC.mm
INCS= LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql
QT5CFLAGS!= pkg-config --cflags Qt5Xdg Qt5DBus
QT5LDFLAGS!= pkg-config --libs Qt5Xdg Qt5DBus
CFLAGS+= ${FMWK_CFLAGS} -fobjc-arc -I.. -I../libobjc2 -I../Foundation/Headers ${QT5CFLAGS}
LDFLAGS+= -L/usr/local/lib -lsqlite3 ${QT5LDFLAGS} -llaunch -lmach

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

installresources: _install-hook
