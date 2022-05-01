FRAMEWORK=Cocoa
SRCS=Cocoa.m
INCS=Cocoa.h

RESOURCES=Info.plist
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../Foundation/Headers -I../AppKit/Headers -F${.CURDIR}/../OpenGL \
        -framework OpenGL -I../libobjc2
LDFLAGS+= -L${BUILDROOT}/System/Library/Frameworks/AppKit.framework/Versions/Current \
        -L${BUILDROOT}/System/Library/Frameworks/Foundation.framework/Versions/Current \
        -L${BUILDROOT}/System/Library/Frameworks/CoreFoundation.framework/Versions/Current \
        -lAppKit -lFoundation -lCoreFoundation -lobjc -lmach \
        -Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A 

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <rvn.framework.mk>

installresources: _install-hook
