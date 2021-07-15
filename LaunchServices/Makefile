.include <internal/framework.common.mk>

FRAMEWORK=LaunchServices
SRCS= LaunchServices.mm LSAppRecord.mm
INCS= LaunchServices.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../libobjc2 -I../Foundation/Headers -I/usr/local/include
LDFLAGS+= -L/usr/local/lib -lsqlite3

build: all

.include <airyx.framework.mk>
