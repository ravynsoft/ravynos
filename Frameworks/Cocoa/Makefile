FRAMEWORK=Cocoa
SRCS=Cocoa.m
INCS=Cocoa.h

RESOURCES=Info.plist
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../Foundation/Headers -I../AppKit/Headers -I../OpenGL/include -I../libobjc2

install:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/
_libinstall: install

.include <airyx.framework.mk>
