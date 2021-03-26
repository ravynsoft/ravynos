# Common settings for building frameworks
BASEFONT=\"DejaVu Sans\"
MONOFONT=\"DejaVu Sans Mono\"
FMWK_CFLAGS := -O0 -g -D__HELIUM__ -DBSD -DPLATFORM_IS_POSIX -DGCC_RUNTIME_3 \
	 -DPLATFORM_USES_BSD_SOCKETS -DBASEFONT_SZ="${BASEFONT}" -DBASEFONT_NS="@${BASEFONT}" \
	 -DMONOFONT_SZ="${MONOFONT}" -DMONOFONT_NS="@${MONOFONT}" -DBASEFONT_SIZE=12.0 \
	 -DMONOFONT_SIZE=12.0 \
	 -fobjc-runtime=gnustep-2.0 \
	 -fobjc-nonfragile-abi -fPIC
FMWK_LDFLAGS+= -L${BUILDROOT}/usr/lib -L/usr/lib -Wl,--no-as-needed

