FRAMEWORK=LaunchServices
.include "../framework.common.mk"

SRCS= LaunchServices.mm LSAppRecord.mm UTTypes.mm LSIPC.mm
INCS= LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql
#QT5CFLAGS!= pkg-config --cflags Qt5Xdg Qt5DBus
#QT5LDFLAGS!= pkg-config --libs Qt5Xdg Qt5DBus
CFLAGS+= ${FMWK_CFLAGS} -fobjc-arc -I.. -I../libobjc2 -I../Foundation/Headers
LDFLAGS+= -lsqlite3 -llaunch -lmach

.include <rvn.framework.mk>

installresources: std_install_hook
