.include <internal/framework.common.mk>

FRAMEWORK=DBusKit
SRCS= \
      DKConnection.m DKMessage.m

INCS= \
      DKConnection.h DKMessage.h

RESOURCES=
DBUS_CFLAGS!= pkg-config --cflags dbus-1
DBUS_LDFLAGS!= pkg-config --libs dbus-1
CFLAGS+= -g ${FMWK_CFLAGS} -I${.CURDIR}/.. -I${BUILDROOT}/usr/include -I/usr/local/include \
	 -I. -I../Foundation/Headers ${DBUS_CFLAGS} -fblocks
LDFLAGS+= ${FMWK_LDFLAGS} -L/usr/local/lib ${DBUS_LDFLAGS} \
	  -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	  -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A \
	  -L${BUILDROOT}/System/Library/Frameworks/Foundation.framework/Versions/Current \
	  -L${BUILDROOT}/System/Library/Frameworks/CoreFoundation.framework/Versions/Current \
	  -lFoundation -lCoreFoundation -lobjc -lm

build: all

.include <he.framework.mk>
