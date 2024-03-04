.include <src.opts.mk>

PACKAGE=        wayland
MAJOR=  	1
MINOR=  	22
MICRO=  	0

SUBDIR=		cursor egl src

CFLAGS+=	-I${OBJTOP}/lib/libepoll-shim/include
LDFLAGS+=	-L${OBJTOP}/lib/libepoll-shim -lepoll-shim -lffi -lbsdxml -lthr
#LD_LIBRARY_PATH="${OBJTOP}/tmp/lib:${OBJTOP}/tmp/usr/lib:${OBJTOP}/lib/libepoll-shim"

.export PACKAGE MAJOR MINOR MICRO CFLAGS LDFLAGS
.include <bsd.subdir.mk>
