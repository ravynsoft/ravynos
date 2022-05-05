FRAMEWORK=CoreServices
SRCS=
INCS=CoreServices.h 

RESOURCES=Info.plist English.lproj

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <rvn.framework.mk>

installresources: _install-hook
