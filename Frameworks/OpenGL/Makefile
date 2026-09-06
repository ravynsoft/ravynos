FRAMEWORK = OpenGL

SUBDIR =
MK_INSTALLLIB = no
MK_WERROR = no
SRCS =	OpenGL.c
HEADERS = \
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
RESOURCES = Info.plist

CFLAGS = -I${ROOT_SOURCE_DIR}/Frameworks -I${.CURDIR}/include \
	-I${ROOT_SOURCE_DIR}/Frameworks/Foundation/Headers

fmwk-install-hook:
.for dir in ${RAVYN_SDKROOT}/System/Library/Frameworks/ ${SYSROOT_DIR}/System/Library/Frameworks/
	mkdir -p ${dir}
	cp -Rf ${.OBJDIR}/${FRAMEWORK}.framework ${dir}
.endfor

.include <rvn.framework.mk>
