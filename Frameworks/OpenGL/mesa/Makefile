.include <bsd.lib.mk>

LIB=            libmesa
PACKAGE=        OpenGL

CFLAGS=         --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
CFLAGS+=        -DHAVE_DRISW_KMS -DHAVE_DRI -DHAVE_DRI2 -DHAVE_LIBDRM
CFLAGS+=        -I${.CURDIR}/../wayland/src
CFLAGS+=        -I${.CURDIR}/../wayland/egl
CFLAGS+=        -I${MAKEOBJDIR}/../wayland/src
CFLAGS+=        -I${.CURDIR}/../libdrm
CFLAGS+=        -I${.CURDIR}/../libdrm/amdgpu
CFLAGS+=        -I${.CURDIR}/../libdrm/nouveau
CFLAGS+=        -I${.CURDIR}/../libdrm/radeon
CFLAGS+=        -I${.CURDIR}/../libdrm/intel
CFLAGS+=        -I${.CURDIR}/../libdrm/include/drm
CFLAGS+=        -Wno-format -Wno-asm-operand-widths

LDFLAGS=        -L${MAKEOBJDIR}/../wayland/src -lwayland
LDFLAGS+=       -L${MAKEOBJDIR}/../libdrm -ldrm
LDFLAGS+=       -Wl,-R\$$ORIGIN

MESON?=         ${OBJTOP}/tmp/usr/bin/meson
NINJA?=         ${OBJTOP}/tmp/usr/bin/ninja

all: build
build:
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib:${OBJTOP}/Frameworks/OpenGL/libpciaccess:${OBJTOP}/Frameworks/OpenGL/wayland/src:${OBJTOP}/Frameworks/OpenGL/libdrm \
        PYTHONPATH=${OBJTOP}/tmp/usr/lib/python3.10 \
        PATH=${OBJTOP}/tmp/usr/bin:${OBJTOP}/tmp/legacy/bin:${OBJTOP}/Frameworks/OpenGL/wayland/scanner:$${PATH} \
	CFLAGS="${CFLAGS:C/-Werror=.* //g}" \
        ${MESON} setup --reconfigure -Dprefix=${PREFIX} -Dplatforms=wayland \
        -DSRCTOP=${SRCTOP} -Dc_args="${CFLAGS}" -Dcpp_args="${CFLAGS}" \
        -Dc_link_args="${LDFLAGS}" -Dcpp_link_args="${LDFLAGS}" \
        -Degl-native-platform=drm -Dgallium-drivers='i915,iris,svga,swrast' \
        -Dgallium-vdpau=disabled -Dvulkan-drivers='[]' -Dllvm=disabled \
        -Dgles1=enabled -Dgles2=enabled -Dopengl=true \
        -Degl=enabled -Dlibunwind=disabled -Dlmsensors=disabled -Dbuild-tests=false \
        -Dzstd=disabled -Dzlib=enabled -Dvideo-codecs=all -Ddri3=disabled \
        -Dgbm=enabled -Dxmlconfig=disabled -Dexpat=disabled \
	-Ddri-search-path=/System/Library/Frameworks/OpenGL.framework/Versions/Current:/Library/Drivers/DRI:/usr/lib/dri \
	-Dgbm-backends-path=/System/Library/Frameworks/OpenGL.framework/Versions/Current:/Library/Drivers/GBM:/usr/lib/gbm \
	${MAKEOBJDIR} ${.CURDIR}
	sed -i.bak -e 's@-I/usr/include@@g' ${MAKEOBJDIR}/build.ninja
	PATH=${OBJTOP}/tmp/usr/bin:${OBJTOP}/tmp/legacy/bin:${OBJTOP}/Frameworks/OpenGL/wayland/scanner:${OBJTOP}/usr.bin/clang/llvm-ar:$$PATH \
	${NINJA}

install:
	echo ${PACKAGE} ${LIB} ${.TARGET}
	#${MESON} install --destdir=${DESTDIR} --no-rebuild

_install-mesa-deps:
	echo ${PACKAGE} ${.TARGET}
	cp -vf ../wayland/src/libwayland.so.0 ../libdrm/libdrm.so.2 \
            ../${PACKAGE}.framework/Versions/A/
	ln -sf libwayland.so.0 ../${PACKAGE}.framework/Versions/A/libwayland.so
	ln -sf libdrm.so.2 ../${PACKAGE}.framework/Versions/A/libdrm.so
	mkdir -p meson-private
	rm -f meson-private/Versions
	ln -sv ${MAKEOBJDIR}/../${PACKAGE}.framework/Versions/ meson-private/Versions

beforebuild: obj
beforebuild: _install-mesa-deps

