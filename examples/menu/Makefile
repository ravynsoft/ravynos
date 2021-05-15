APP=Menu
SRCS=main.m
MK_DEBUG_FILES=no
RESOURCES=CocoaDemo.png
SLF=/System/Library/Frameworks
FRAMEWORKS=/Users/zoe/helium/AppKit/AppKit ${SLF}/CoreGraphics ${SLF}/Onyx2D ${SLF}/OpenGL \
        /Users/zoe/helium/Foundation/Foundation ${SLF}/CoreFoundation /Users/zoe/helium/DBusKit/DBusKit
LDFLAGS+=-L/usr/local/lib -lGL
CFLAGS+= -g

build: all

.include <he.app.mk>
