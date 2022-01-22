CIRRUS_BUILD_ID?= local

SUBDIR=		CoreFoundation
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
	sed "s/%%VERSION%%/${AIRYX_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	mkdir -p ${TOPDIR}/dist
	pkg create -o ${TOPDIR}/dist/ -r ${BUILDROOT} -m /tmp/meta \
	  --plist /tmp/meta/pkg-plist
	ls -l ${TOPDIR}/dist
