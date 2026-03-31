CMAKE?= ${OBJTOP}/tmp/usr/bin/cmake
MAKEOBJDIR= ${OBJTOP}/usr.bin/curl

all: curl
curl:
	${CMAKE} -S${SRCTOP}/contrib/curl -B${OBJTOP}/usr.bin/curl \
	-DUSE_APPLE_IDN=NO -DCMAKE_INSTALL_PREFIX=${PREFIX} \
	-DCURL_DISABLE_LDAP=YES -DCURL_USE_LIBPSL=NO -GNinja
	cd ${OBJTOP}/usr.bin/curl && ninja
	${CMAKE} --install ${OBJTOP}/usr.bin/curl --prefix=${BUILDROOT}/usr

clean:
	rm -rf ${OBJTOP}/usr.bin/curl