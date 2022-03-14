.include "../framework.common.mk"

FRAMEWORK=OpenGL
SRCS= OpenGL.m
INCS= \
	include/CGLCurrent.h \
	include/CGLTypes.h \
	include/gl.h \
	include/glext.h \
	include/glplatform.h \
	include/OpenGL.h \
        include/egl.h \
        include/eglext.h \
        include/eglextchromium.h \
        include/eglexternalplatform.h \
        include/eglexternalplatformversion.h \
        include/eglmesaext.h \
        include/eglplatform.h \
        include/khrplatform.h
RESOURCES=Info.plist
CFLAGS+= ${FMWK_CFLAGS} -I.. -Iinclude -I../libobjc2 -I../Foundation/Headers
LDFLAGS+= -lGLESv2 -lEGL -Wl,--no-as-needed

_install-hook:
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${.CURDIR}/${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

.include <airyx.framework.mk>

installresources: _install-hook
