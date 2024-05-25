FRAMEWORK=OpenGL
SUBDIR+=        libpciaccess
SUBDIR+=        libdrm 
SUBDIR+=        wayland .WAIT
SUBDIR+=        mesa

.include "../framework.common.mk"

MK_INSTALLLIB= no
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
RESOURCES=      Info.plist

LINK_SUBDIR+= mesa/src/egl
LINK_SUBDIR+= mesa/src/mapi/es2api
LINK_SUBDIR+= mesa/src/mapi/shared-glapi
LINK_SUBDIR+= mesa/src/gbm
LINK_SUBDIR+= mesa/src/util

CFLAGS+= ${FMWK_CFLAGS} -I${SRCTOP}/Frameworks -I${.CURDIR}/include 
CFLAGS+= -I${SRCTOP}/Frameworks/Foundation/Headers
CFLAGS+= -I${.CURDIR}/libpciaccess/include
LDFLAGS+= -Llibdrm -Llibpciaccess -Lwayland/src \
	  -lwayland -ldrm -lpciaccess -lstdc++ \
	  -Wl,--no-as-needed -Wl,-R\$$ORIGIN

_libinstall_hook:
	echo ${FRAMEWORK} ${.TARGET}
	rm -rf mesa/src/gallium/targets/dri/libgallium_dri.so*.p
	cp -Rfv mesa/src/gallium/targets/dri/libgallium_dri.so \
		${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
.for i in \
          src/gallium/targets/xa/libxatracker.so.2 \
          src/mapi/es1api/libGLESv1_CM.so.1 
	rm -rf mesa/${i}*.p
	cp -Rfv mesa/${i}* ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	ln -sfv ${i:T} ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/${i:T:R}
.endfor
	cp -fv libpciaccess/libpciaccess.so.0 ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	cp -fv libOpenGL.so ${FRAMEWORK}.framework/Versions/Current/
.for inc in ${INCS}
	cp -fv ${.CURDIR}/${inc} ${FRAMEWORK}.framework/Headers/
.endfor


.include <rvn.framework.mk>

installresources: std_install_hook
_libinstall: _libinstall_hook
