.include <bsd.lib.mk>

LIB=            libmesa
PACKAGE=        OpenGL

all: build
build:
	meson setup -Dprefix=${PREFIX} -Dplatforms=wayland \
        -DSRCTOP=${SRCTOP} -Dc_cpp_args="${NEWCFLAGS}" \
        -Degl-native-platform=wayland -Dgallium-drivers=auto \
        -Dgallium-vdpau=disabled -Dvulkan-drivers="[]" -Dllvm=disabled \
        -Dgles1=enabled -Dgles2=enabled -Dopengl=false -Dglx=disabled \
        -Degl=enabled -Dlibunwind=disabled -Dlmsensors=disabled -Dbuild-tests=false \
        -Dzstd=disabled -Dzlib=enabled -Dvideo-codecs=all ${MAKEOBJDIR} ${.CURDIR}
	sed -i.bak -e 's@-I/usr/include@@g' ${MAKEOBJDIR}/build.ninja
	ninja

install:
	${MAKE} -C ${MAKEOBJDIR} DESTDIR=${DESTDIR} install
