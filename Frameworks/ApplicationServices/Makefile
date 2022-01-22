FRAMEWORK=ApplicationServices
SRCS=
INCS=ApplicationServices.h 

RESOURCES=Info.plist

install:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/
_libinstall: install

.include <airyx.framework.mk>
