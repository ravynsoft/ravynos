MK_AUTO_OBJ=    yes
beforebuild: make-obj-dirs obj

.include <sys.mk>
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
RESOURCES=      Info.plist

CFLAGS+= ${FMWK_CFLAGS} -I.. -Iinclude -I../libobjc2 -I../Foundation/Headers
LDFLAGS+= -Lmesa/src/egl -Lmesa/src/mapi/es2api -Lmesa/src/mapi/es1api \
          -Lmesa/src/mapi/shared-glapi -Lwayland/src -Lmesa/src/gbm \
          -Llibdrm -lEGL -lGLESv2 -lGLESv1_CM -lglapi -lwayland -lgbm -ldrm \
          -Wl,--no-as-needed -Wl,-R\$$ORIGIN/Resources

SUBDIR+=        libpciaccess
SUBDIR+=        libdrm 
SUBDIR+=        wayland .WAIT
SUBDIR+=        mesa


_install-hook:
	echo ${FRAMEWORK} ${.TARGET}
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

_libinstall:
	echo ${FRAMEWORK} ${.TARGET}
.for i in subprojects/expat-2.5.0/libexpat.so.1 \
          src/gallium/targets/dri/libgallium_dri.so \
          src/gallium/targets/xa/libxatracker.so.2 \
          src/mapi/es2api/libGLESv2.so.2 \
          src/mapi/es1api/libGLESv1_CM.so.1 \
          src/mapi/shared-glapi/libglapi.so.0 \
          src/gbm/libgbm.so.1 \
          src/egl/libEGL.so.1 
	rm -rf mesa/${i}*.p
	cp -Rfv mesa/${i}* ${FRAMEWORK}.framework/Resources/
.endfor
	cp -fv libpciaccess/libpciaccess.so.1 ${FRAMEWORK}.framework/Resources/
	cp -fv libOpenGL.so ${FRAMEWORK}.framework/Versions/Current/

make-obj-dirs: ${FRAMEWORK}.framework
	mkdir -p ${OBJTOP}/Frameworks/${FRAMEWORK}
.for subdir in ${SUBDIR}
	mkdir -p ${OBJTOP}/Frameworks/${FRAMEWORK}/${subdir}
.endfor

.include <bsd.subdir.mk>
.include <rvn.framework.mk>

installresources: _install-hook
