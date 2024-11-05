THISDIR:= ${.CURDIR}
.if "${CIRRUS_CI}"
SHA!= echo ${CIRRUS_CHANGE_IN_REPO}|cut -c1-7|tr '[a-z]' '[A-Z]'
.else
SHA!= cd ${.CURDIR} && git log -1|head -1|cut -c8-14|tr '[a-z]' '[A-Z]'
.endif

all: clean build
clean:
	rm -rf ${BUILDROOT}/Applications/Utilities/"Install ravynOS.app"
	rm -rf Installer/"Install ravynOS.app" ${MAKEOBJDIR}/Installer
	rm -f ${OBJTOP}/SysApps/grabscr ${BUILDROOT}/usr/bin/grabscr

build: buildInstaller grabscr
	mkdir -p ${BUILDROOT}/Applications/Utilities/
	mkdir -p ${BUILDROOT}/usr/bin
	cp -av ${OBJTOP}/SysApps/grabscr ${BUILDROOT}/usr/bin
	#cp -av ${MAKEOBJDIR}/Installer/"Install ravynOS.app" ${BUILDROOT}/Applications/Utilities/

buildInstaller:
	#${MAKE} -C ${.CURDIR}/Installer build

grabscr: grabscr.m
	mkdir -p ${OBJTOP}/SysApps
	${CC} -B${OBJTOP}/tmp/usr/bin --sysroot=${OBJTOP}/tmp \
		-g -o ${MAKEOBJDIR}/${.TARGET} ${.ALLSRC} \
		-I${OBJTOP}/Frameworks/AppKit/Headers \
		-I${OBJTOP}/Frameworks/Foundation/Headers \
		-F${OBJTOP}/Frameworks/OpenGL \
		-I${SRCTOP}/Frameworks -L${OBJTOP}/Frameworks/AppKit/AppKit.framework \
		-L${OBJTOP}/Frameworks/CoreGraphics/CoreGraphics.framework \
		-lAppKit -lCoreGraphics -lobjc \
		-Wl,-R/System/Library/Frameworks/AppKit.framework/Versions/A \
		-Wl,-R/System/Library/Frameworks/CoreGraphics.framework/Versions/A
