CIRRUS_BUILD_ID?= local

all: .PHONY
build: .PHONY
install:
	rm -rf /tmp/meta
	mkdir -p /tmp/meta
	cd ${BUILDROOT}; find ./usr/bin -not -type d | sed 's@^\.@@' > /tmp/meta/pkg-plist
	sed "s/%%VERSION%%/${RAVYNOS_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	mkdir -p ${TOPDIR}/dist
	pkg create -o ${TOPDIR}/dist/ -r ${BUILDROOT} -m /tmp/meta \
	  --plist /tmp/meta/pkg-plist
	ls -l ${TOPDIR}/dist
