JAVA_VER= 17
SRCDIR= ${MAKEOBJDIR}/jdk${JAVA_VER}u
INSTDIR= /Library/Java/JavaVirtualMachines/openjdk-${JAVA_VER}.0.1.jdk
BUNDLEDIR= ${BUILDROOT}${INSTDIR}
TAG= jdk-17.0.2+8-1

all: build install

${SRCDIR}:
	git clone https://github.com/battleblow/jdk17u.git

build: ${SRCDIR}
	#cd ${SRCDIR} && git checkout ${TAG}
	cd ${SRCDIR} && bash configure --with-boot-jdk=${INSTDIR}/Contents/Home \
	  --with-toolchain-type=clang --enable-jvm-feature-dtrace=no \
	  --with-version-opt="ravynos" --with-version-pre="" --with-version-build=8
	gmake -C ${SRCDIR}

install:
	rm -rf ${BUNDLEDIR}
	mkdir -p ${BUNDLEDIR}/Contents/Home
	cd ${SRCDIR}/build/bsd-x86_64-server-release/jdk && \
	  cp -a bin conf include lib man modules release \
	  ${BUNDLEDIR}/Contents/Home/
	cd ${SRCDIR}/build/bsd-x86_64-server-release && \
	  cp -a support ${BUNDLEDIR}/Contents/
	mkdir -p ${BUNDLEDIR}/Contents/ravynOS
	ln -sf ../Home/lib/libjli.so ${BUNDLEDIR}/Contents/ravynOS/libjli.so
	find ${BUNDLEDIR} -name \*.debuginfo -exec rm {} \;
	# create Info.plist in Contents

clean:
	if [ -d ${SRCDIR} ]; then gmake -C ${SRCDIR} clean; fi

