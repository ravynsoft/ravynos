.include <src.opts.mk>

MAJOR=          0
MINOR=          8
MICRO=          0
CFLAGS+= -D_XOPEN_SOURCE=700 -D__BSD_VISIBLE -D_NETBSD_SOURCE \
         -D_FORTIFY_SOURCE=2 -DSEATD_VERSION=\"${MAJOR}.${MINOR}.${MICRO}\" \
	 -DSEATD_DEFAULTPATH=\"/var/run/seatd.sock\" \
	 -DSEATD_INSTALLPATH=\"/System/Library/CoreServices/WindowServer/Versions/Current/Resources\" \
         -Wl,-R\$$ORIGIN/Resources -Wl,--exclude-libs=ALL
.export CFLAGS MAJOR MINOR MICRO

SUBDIR=	libseat .WAIT seatd

.include <bsd.subdir.mk>
