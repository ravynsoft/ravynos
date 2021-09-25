JAVA_VER= 17
SRCDIR= ${.CURDIR}/jdk${JAVA_VER}u
INSTDIR= /Library/Java/JavaVirtualMachines/openjdk-${JAVA_VER}.jdk
BUNDLEDIR= ${BUILDROOT}${INSTDIR}

all: build install

${SRCDIR}:
	git clone https://github.com/battleblow/jdk17u.git

build: ${SRCDIR}
	cd ${SRCDIR} && bash configure --with-boot-jdk=${INSTDIR}/Contents/Home \
	  --with-toolchain-type=clang --with-source-date="2021-09-14" \
	  --with-version-opt="airyx" --with-version-pre=""
	gmake -C ${SRCDIR}

install:
	rm -rf ${BUNDLEDIR}
	mkdir -p ${BUNDLEDIR}/Contents/Home
	cd ${SRCDIR}/build/bsd-x86_64-server-release/jdk && \
	  cp -a bin conf include lib man modules release \
	  ${BUNDLEDIR}/Contents/Home/
	cd ${SRCDIR}/build/bsd-x86_64-server-release && \
	  cp -a support ${BUNDLEDIR}/Contents/
	mkdir -p ${BUNDLEDIR}/Contents/Airyx
	ln -sf ../Home/lib/libjli.so ${BUNDLEDIR}/Contents/Airyx/libjli.so
	find ${BUNDLEDIR} -name \*.debuginfo -exec rm {} \;
	# create Info.plist in Contents

clean:
	if [ -d ${SRCDIR} ]; then gmake -C ${SRCDIR} clean; fi

