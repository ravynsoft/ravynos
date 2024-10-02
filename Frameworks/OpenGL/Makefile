FRAMEWORK=OpenGL
SUBDIR+=        libpciaccess
SUBDIR+=        libdrm 

.include "../framework.common.mk"

MK_INSTALLLIB= no
SRCS=	OpenGL.c 
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
RESOURCES=      Info.plist

CFLAGS+= ${FMWK_CFLAGS} -I${SRCTOP}/Frameworks -I${.CURDIR}/include 
CFLAGS+= -I${SRCTOP}/Frameworks/Foundation/Headers
CFLAGS+= -I${.CURDIR}/libpciaccess/include
LDFLAGS+= -Llibdrm -Llibpciaccess -ldrm -lpciaccess -lstdc++ -lstdthreads \
	  -Wl,--no-as-needed -Wl,-R\$$ORIGIN
LD_LIBRARY_PATH+= ${OBJTOP}/tmp/usr/lib
.export LD_LIBRARY_PATH

_libinstall_hook:
	echo ${FRAMEWORK} ${.TARGET}
	cp -fv libpciaccess/libpciaccess.so.0 ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	cp -fv libpciaccess/libdrm.so.2 ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
.for inc in ${INCS}
	cp -fv ${.CURDIR}/${inc} ${FRAMEWORK}.framework/Headers/
.endfor


.include <rvn.framework.mk>

installresources: std_install_hook
_libinstall: _libinstall_hook
