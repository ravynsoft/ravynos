.include "../framework.common.mk"

MK_WERROR= no
FRAMEWORK=CoreVideo
SRCS= CVDisplayLink.m
INCS= CVBase.h CVDisplayLink.h CVReturn.h CoreVideo.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -I../OpenGL/include -I${.CURDIR}/.. \
	-I../libobjc2 -I../Foundation/Headers

install:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/
_libinstall: install

.include <airyx.framework.mk>
