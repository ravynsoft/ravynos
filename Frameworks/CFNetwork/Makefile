.include "../framework.common.mk"

FRAMEWORK=CFNetwork
SRCS= \
	CFHost.m \
	CFNetServices.m \
	CFSocketStream.m

INCS= \
	CFHost.h \
	CFNetServices.h \
	CFNetwork.h \
	CFNetworkExport.h \
	CFSocketStream.h

RESOURCES=Info.plist English.lproj
CFLAGS+= ${FMWK_CFLAGS} -I.. -I../Foundation/Headers \
	-F${BUILDROOT}/System/Library/Frameworks -framework CoreFoundation
LDFLAGS+=-L${BUILDROOT}/System/Library/Frameworks/CoreFoundation.framework/Versions/Current -lCoreFoundation -Wl,-R/System/Library/Frameworks/CoreFoundation.framework/Versions/A

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

installresources: _install-hook
