CIRRUS_BUILD_ID?= local

all: .PHONY
build: .PHONY
install:
	tar -C ${BUILDROOT} -cf - . | ${SUDO} tar -C / -xvf -
	rm -rf /tmp/meta
	mkdir -p /tmp/meta
	find ${BUILDROOT}/usr/bin -not -type d | \
	  sed 's@${BUILDROOT}@@' > /tmp/meta/pkg-plist
	sed "s/%%VERSION%%/${AIRYX_VERSION}_${CIRRUS_BUILD_ID}/" < +MANIFEST \
	  > /tmp/meta/+MANIFEST
	pkg create -o ${TOPDIR}/dist -m /tmp/meta --plist /tmp/meta/pkg-plist
