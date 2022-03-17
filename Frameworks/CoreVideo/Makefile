.include "../framework.common.mk"

MK_WERROR= no
FRAMEWORK=CoreVideo
SRCS= CVDisplayLink.m
INCS= CVBase.h CVDisplayLink.h CVReturn.h CoreVideo.h
RESOURCES=Info.plist 
CFLAGS+= ${FMWK_CFLAGS} -F${.CURDIR}/../OpenGL -framework OpenGL -I${.CURDIR}/.. \
	-I../libobjc2 -I../Foundation/Headers

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

installresources: _install-hook
