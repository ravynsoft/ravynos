# Common settings for building frameworks
FMWK_CFLAGS := -O0 -g -D__HELIUM__ -DBSD -DPLATFORM_IS_POSIX -DGCC_RUNTIME_3 \
	 -DPLATFORM_USES_BSD_SOCKETS -fobjc-runtime=gnustep-2.0 \
	 -fobjc-nonfragile-abi -fPIC
FMWK_LDFLAGS+= -L${BUILDROOT}/usr/lib -L/usr/lib -lobjc -lunwind -lpthread

