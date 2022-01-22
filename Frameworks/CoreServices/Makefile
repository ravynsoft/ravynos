FRAMEWORK=CoreServices
SRCS=
INCS=CoreServices.h 

RESOURCES=Info.plist English.lproj

install:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/
_libinstall: install

.include <airyx.framework.mk>
