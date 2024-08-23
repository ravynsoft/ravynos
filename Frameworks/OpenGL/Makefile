FRAMEWORK=OpenGL
SUBDIR+=        libpciaccess
SUBDIR+=        libdrm 
SUBDIR+=        wayland .WAIT
SUBDIR+=        mesa

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

LINK_SUBDIR+= mesa/src/egl
LINK_SUBDIR+= mesa/src/mapi/es2api
LINK_SUBDIR+= mesa/src/mapi/shared-glapi
LINK_SUBDIR+= mesa/src/gbm
LINK_SUBDIR+= mesa/src/util
LINK_SUBDIR+= mesa/src/loader

CFLAGS+= ${FMWK_CFLAGS} -I${SRCTOP}/Frameworks -I${.CURDIR}/include 
CFLAGS+= -I${SRCTOP}/Frameworks/Foundation/Headers
CFLAGS+= -I${.CURDIR}/libpciaccess/include
LDFLAGS+= -Llibdrm -Llibpciaccess -Lwayland/src \
	  -lwayland -ldrm -lpciaccess -lstdc++ -lstdthreads \
	  -Wl,--no-as-needed -Wl,-R\$$ORIGIN
LD_LIBRARY_PATH+= ${OBJTOP}/tmp/usr/lib
.export LD_LIBRARY_PATH

_libinstall_hook:
	echo ${FRAMEWORK} ${.TARGET}
.for i in \
          src/mapi/es1api/libGLESv1_CM.so.1 \
	  src/mapi/shared-glapi/libglapi.so.0 \
	  src/mapi/es2api/libGLESv2.so.2 \
	  src/egl/libEGL.so.1 \
	  src/gbm/libgbm.so.1 \
	  src/gallium/targets/xa/libxatracker.so.2
	rm -rf mesa/${i}*.p
	cp -Rfv mesa/${i}* ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
	ln -sfv ${i:T} ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/${i:T:R}
.endfor
.for i in \
	  src/gallium/targets/dri/i915_dri.so \
	  src/gallium/targets/dri/iris_dri.so \
	  src/gallium/targets/dri/kms_swrast_dri.so \
	  src/gallium/targets/dri/libgallium_dri.so \
	  src/gallium/targets/dri/swrast_dri.so \
	  src/gallium/targets/dri/vmwgfx_dri.so
	cp -Rfv mesa/${i} ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
.endfor
	cp -fv libpciaccess/libpciaccess.so.0 ${FRAMEWORK}.framework/Versions/${FMWK_VERSION}/
.for inc in ${INCS}
	cp -fv ${.CURDIR}/${inc} ${FRAMEWORK}.framework/Headers/
.endfor


.include <rvn.framework.mk>

installresources: std_install_hook
_libinstall: _libinstall_hook
