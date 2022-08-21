CIRRUS_BUILD_ID?= local
MK_WERROR= no
WARN?= 1
WORLDTMP= /usr/obj/${TOPDIR}/${MACHINE}.${MACHINE_CPUARCH}
CFLAGS+= -I ${WORLDTMP}/tmp/usr/include
#CFLAGS+=	-B ${WORLDTMP}/usr/bin
LDFLAGS+= -B ${WORLDTMP}/usr/bin \
	-L ${WORLDTMP}/tmp/lib -L ${WORLDTMP}/tmp/usr/lib

.export WARN MK_WERROR CFLAGS LDFLAGS

SUBDIR+=	setup .WAIT
SUBDIR+=	CoreFoundation
SUBDIR+=	CFNetwork
SUBDIR+=	Foundation
SUBDIR+=	ApplicationServices
SUBDIR+=	CoreServices
SUBDIR+=	CoreData
SUBDIR+=	Onyx2D
SUBDIR+=	OpenGL
SUBDIR+=	CoreGraphics
SUBDIR+=	CoreText
SUBDIR+=	CoreVideo
SUBDIR+=	QuartzCore
SUBDIR+=	AppKit
SUBDIR+=	DBusKit
SUBDIR+=	Cocoa
SUBDIR+=	LaunchServices

.include <bsd.subdir.mk>

package:
	rm -rf /tmp/meta
	mkdir -p /tmp/meta
	cd ${BUILDROOT}; find ./System/Library/Frameworks -not -type d | \
	  sed 's@^\.@@' > /tmp/meta/pkg-plist
	sed "s/%%VERSION%%/${RAVYNOS_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	mkdir -p ${TOPDIR}/dist
	pkg create -o ${TOPDIR}/dist/ -r ${BUILDROOT} -m /tmp/meta \
	  --plist /tmp/meta/pkg-plist
	ls -l ${TOPDIR}/dist
