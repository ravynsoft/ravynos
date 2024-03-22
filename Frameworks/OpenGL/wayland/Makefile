.include <src.opts.mk>

PACKAGE=        wayland
MAJOR=  	1
MINOR=  	22
MICRO=  	0

SUBDIR=		cursor egl src scanner

CFLAGS+=	-I${OBJTOP}/lib/libepoll-shim/include
LDFLAGS+=	-L${OBJTOP}/lib/libepoll-shim -lepoll-shim -lffi -lbsdxml -lthr

.export PACKAGE MAJOR MINOR MICRO CFLAGS LDFLAGS
.include <bsd.subdir.mk>
.include <bsd.lib.mk>
