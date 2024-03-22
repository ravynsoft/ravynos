.include <bsd.lib.mk>

LIB=            libmesa
PACKAGE=        OpenGL

CFLAGS=         --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
CFLAGS+=        -I${.CURDIR}/../wayland/src
CFLAGS+=        -I${.CURDIR}/../wayland/egl
CFLAGS+=        -I${MAKEOBJDIR}/../wayland/src
CFLAGS+=        -I${.CURDIR}/../libdrm
CFLAGS+=        -I${.CURDIR}/../libdrm/amdgpu
CFLAGS+=        -I${.CURDIR}/../libdrm/nouveau
CFLAGS+=        -I${.CURDIR}/../libdrm/radeon
CFLAGS+=        -I${.CURDIR}/../libdrm/intel
CFLAGS+=        -I${.CURDIR}/../libdrm/include/drm

LDFLAGS=        -L${MAKEOBJDIR}/../wayland/src -lwayland
LDFLAGS+=       -L${MAKEOBJDIR}/../libdrm -ldrm
LDFLAGS+=       -Wl,-R\$$ORIGIN/Resources/

MESON?=         ${OBJTOP}/tmp/usr/bin/meson
NINJA?=         ${OBJTOP}/tmp/usr/bin/ninja

all: build
build:
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib \
        PYTHONPATH=${OBJTOP}/tmp/usr/lib/python3.10 \
        PATH=${OBJTOP}/tmp/usr/bin:${OBJTOP}/tmp/legacy/bin:${PATH} \
        ${MESON} setup --reconfigure -Dprefix=${PREFIX} -Dplatforms=wayland \
        -DSRCTOP=${SRCTOP} -Dc_args="${CFLAGS}" -Dcpp_args="${CFLAGS}" \
        -Dc_link_args="${LDFLAGS}" -Dcpp_link_args="${LDFLAGS}" \
        -Degl-native-platform=wayland -Dgallium-drivers=auto \
        -Dgallium-vdpau=disabled -Dvulkan-drivers="[]" -Dllvm=disabled \
        -Dgles1=enabled -Dgles2=enabled -Dopengl=false -Dglx=disabled \
        -Degl=enabled -Dlibunwind=disabled -Dlmsensors=disabled -Dbuild-tests=false \
        -Dzstd=disabled -Dzlib=enabled -Dvideo-codecs=all ${MAKEOBJDIR} ${.CURDIR}
	sed -i.bak -e 's@-I/usr/include@@g' ${MAKEOBJDIR}/build.ninja
	${NINJA}

install:
	echo ${PACKAGE} ${LIB} ${.TARGET}
	#${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
