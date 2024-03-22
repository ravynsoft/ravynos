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
LDFLAGS+= -lGLESv2 -lEGL -Wl,--no-as-needed -Wl,-R\$$ORIGIN/Resources/

SUBDIR+=        libdrm
SUBDIR+=        wayland
SUBDIR+=        mesa

all_subdir_mesa: ${FRAMEWORK}.framework _install-mesa-deps

_install-mesa-deps:
	echo ${FRAMEWORK} ${.TARGET}
	cp -vf ${MAKEOBJDIR}/wayland/src/libwayland.so.0 \
	${MAKEOBJDIR}/libdrm/libdrm.so.2 ${FRAMEWORK}.framework/Resources/
	ln -sf libwayland.so.0 ${FRAMEWORK}.framework/Resources/libwayland.so
	ln -sf libdrm.so.2 ${FRAMEWORK}.framework/Resources/libdrm.so
	mkdir -p mesa/meson-private
	rm -f mesa/meson-private/Resources
	ln -sv ${MAKEOBJDIR}/${FRAMEWORK}.framework/Resources/ mesa/meson-private/Resources

_install-hook:
	echo ${FRAMEWORK} ${.TARGET}
	rm -rf ${BUILDROOT}/System/Library/Frameworks/${FRAMEWORK}.framework
	cp -av ${FRAMEWORK}.framework ${BUILDROOT}/System/Library/Frameworks/

_libinstall:
	echo ${FRAMEWORK} ${.TARGET}

.include <bsd.subdir.mk>
.include <rvn.framework.mk>

installresources: _install-hook
