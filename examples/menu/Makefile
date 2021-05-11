APP=Menu
SRCS=main.m
MK_DEBUG_FILES=no
RESOURCES=
FRAMEWORKS=/Users/zoe/helium/DBusKit/DBusKit /System/Library/Frameworks/Foundation
DBUS_CFLAGS!= pkg-config --cflags dbus-1
DBUS_LDFLAGS!= pkg-config --libs dbus-1
CFLAGS+=-g ${DBUS_CFLAGS}
LDFLAGS+=${DBUS_LDFLAGS}

build: all

.include <he.app.mk>
