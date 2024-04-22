MK_AUTO_OBJ=    yes
beforebuild: make-obj-dirs obj

.include <src.opts.mk>
.include <sys.mk>

PACKAGE=        wayland
MAJOR=  	1
MINOR=  	22
MICRO=  	0

SUBDIR=		scanner .WAIT src .WAIT egl cursor

CFLAGS+=	-I${OBJTOP}/lib/libepoll-shim/include
LDFLAGS+=	-L${OBJTOP}/lib/libepoll-shim -lepoll-shim -lffi -lbsdxml -lthr

make-obj-dirs: .PHONY
.for subdir in ${SUBDIR:N.WAIT}
	mkdir -p ${OBJTOP}/Frameworks/OpenGL/${PACKAGE}/${subdir}
.endfor

.export PACKAGE MAJOR MINOR MICRO CFLAGS LDFLAGS
.include <bsd.subdir.mk>
.include <bsd.lib.mk>
