CIRRUS_BUILD_ID?= local

all: .PHONY
build: .PHONY
install:
	rm -rf /tmp/meta
	mkdir -p /tmp/meta
	find ${BUILDROOT}/usr/bin -not -type d | \
	  sed 's@${BUILDROOT}@@' > /tmp/meta/pkg-plist
	sed "s/%%VERSION%%/${AIRYX_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	mkdir -p ${TOPDIR}/dist
	pkg create -o ${TOPDIR}/dist/ -r ${BUILDROOT} -m /tmp/meta \
	  --plist /tmp/meta/pkg-plist
