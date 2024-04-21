FRAMEWORK=Cocoa
.include "../framework.common.mk"

SRCS=Cocoa.m
INCS=Cocoa.h

RESOURCES=Info.plist
CFLAGS+= ${FMWK_CFLAGS} -I../Foundation/Headers \
	-I${.CURDIR}/../AppKit/Headers -F../OpenGL \
        -framework OpenGL
LDFLAGS+= -L../AppKit -L../Foundation -L../CoreFoundation \
        -lAppKit -lFoundation -lCoreFoundation -lobjc -lmach \
        -Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/Foundation.framework/Versions/A \
        -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A 

.include <rvn.framework.mk>

installresources: std_install_hook
