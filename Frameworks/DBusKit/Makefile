.include "../framework.common.mk"

FRAMEWORK=DBusKit
SRCS= \
      DKConnection.m DKMessage.m DKMenu.m

INCS= \
      DKConnection.h DKMessage.h DKMenu.h

RESOURCES=
DBUS_CFLAGS!= pkg-config --cflags dbus-1
DBUS_LDFLAGS!= pkg-config --libs dbus-1
DBUS_LDFLAGS+=-Wl,-R\$$ORIGIN/./
CFLAGS+= -g ${FMWK_CFLAGS} -I. -I.. -I../Foundation/Headers -I../AppKit/Headers \
	-I${BUILDROOT}/usr/include ${DBUS_CFLAGS} -fblocks
LDFLAGS+= -g ${FMWK_LDFLAGS} ${DBUS_LDFLAGS} \
	  -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
	  -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A \
	  -L${BUILDROOT}/System/Library/Frameworks/Foundation.framework/Versions/Current \
	  -L${BUILDROOT}/System/Library/Frameworks/CoreFoundation.framework/Versions/Current \
	  -lFoundation -lCoreFoundation -lobjc -lm

copydbus:
	rm -rf ${FRAMEWORK}.framework/Headers/dbus
	rm -f ${FRAMEWORK}.framework/Versions/Current/libdbus-1.so*
	cp -rvf /usr/include/dbus-1.0/dbus/*.h ${FRAMEWORK}.framework/Headers/
	cp -vf /usr/lib/dbus-1.0/include/dbus/dbus-arch-deps.h ${FRAMEWORK}.framework/Headers/
	sed -i.bak -e 's/#include <dbus/#import <${FRAMEWORK}/g' ${FRAMEWORK}.framework/Headers/dbus*.h
	rm -f ${FRAMEWORK}.framework/Headers/*.bak
	ln -sf /usr/lib/libdbus-1.so* ${FRAMEWORK}.framework/Versions/Current/

install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

all: copydbus
installresources: install-hook
