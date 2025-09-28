.include <bsd.lib.mk>

LIB=            xkeyboard-config
PACKAGE=        AppKit

CFLAGS=         --sysroot=${OBJTOP}/tmp -B${OBJTOP}/tmp/usr/bin
LDFLAGS+=       -Wl,-R\$$ORIGIN

MESON?=         ${OBJTOP}/tmp/usr/bin/meson
NINJA?=         ${OBJTOP}/tmp/usr/bin/ninja

all: build
build:
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib:${OBJTOP}/tmp/lib \
        PYTHONPATH=${OBJTOP}/tmp/usr/lib/python3.10 \
        PATH=${OBJTOP}/tmp/usr/bin:${OBJTOP}/tmp/legacy/bin:$${PATH} \
	CFLAGS="${CFLAGS:C/-Werror=.* //g}" \
        ${MESON} setup --reconfigure -Dprefix=${PREFIX} \
        -Dc_args="${CFLAGS}" -Dcpp_args="${CFLAGS}" \
        -Dc_link_args="${LDFLAGS}" -Dcpp_link_args="${LDFLAGS}" \
	-Dxkb-base=/System/Library/KeyboardLayout -Dcompat-rules=false \
	${MAKEOBJDIR} ${.CURDIR}
	sed -i.bak -e 's@-I/usr/include@@g' ${MAKEOBJDIR}/build.ninja
	LD_LIBRARY_PATH=${OBJTOP}/tmp/usr/lib:${OBJTOP}/tmp/lib:${OBJTOP}/tmp/obj-tools/lib \
        PYTHONPATH=${OBJTOP}/tmp/usr/lib/python3.10 \
	PATH=${OBJTOP}/tmp/usr/bin:${OBJTOP}/tmp/legacy/bin:${OBJTOP}/usr.bin/clang/llvm-ar:$$PATH \
	${NINJA} -C ${MAKEOBJDIR}

install:
	echo ${PACKAGE} ${LIB} ${.TARGET}
.for d in compat geometry keycodes symbols types
	cp -Rfv ${.CURDIR}/${d} ${BUILDROOT}/System/Library/KeyboardLayout/
.endfor
	mkdir -p ${BUILDROOT}/System/Library/KeyboardLayout/rules
	cp -fv ${.CURDIR}/rules/xfree98 ${BUILDROOT}/System/Library/KeyboardLayout/rules/
.for f in base base.lst base.xml base.extras.xml evdev evdev.lst \
	  evdev.xml evdev.extras.xml xkb.dtd 
	cp -fv ${MAKEOBJDIR}/rules/${f} ${BUILDROOT}/System/Library/KeyboardLayout/rules/
.endfor

beforebuild: obj

