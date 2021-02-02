APP_DIR=${APP}.app
NO_ROOT=yes
PROG=${APP_DIR}/Contents/Helium/${APP}
MAN=

UID != id -u
.if defined(UID) && ${UID} != 0
INSTALLFLAGS=-U
.endif

.if defined(FRAMEWORKS) && !empty(FRAMEWORKS)
_LIBDIRS=
_INCDIRS=
_LIBS=
.for fmwk in ${FRAMEWORKS}
_INCDIRS+= -I${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/Helium\/..\//1}.framework/Headers
_RELLIBDIR= ${fmwk}.framework/Versions/Current 
_FWLIBDIR= ${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/Helium\/..\//1}.framework/Versions/Current
_CURVER!= readlink ${_FWLIBDIR}
_VERLIBDIR= ${_RELLIBDIR:S%Current%${_CURVER}%}
_LIBDIRS+= -L${_FWLIBDIR}
_LIBFILES!=echo ${_FWLIBDIR}/*.so
.for lib in ${_LIBFILES}
_LIBS+= -l${lib:C%.*lib%%:C%\.so$%%} \
	-Wl,--rpath=${_VERLIBDIR:tW:S/..\//\$ORIGIN\/..\//1:Q}
.endfor
.endfor
CFLAGS+= ${_INCDIRS}
LDFLAGS+= ${_LIBDIRS} ${_LIBS}
.endif

.if defined(RESOURCES) && !empty(RESOURCES)
RSCDIR=${APP_DIR}/Contents/Resources
installresources: ${RESOURCES}
	tar -C ${.CURDIR} -cf - ${RESOURCES} | tar -C ${RSCDIR} -xvf -
.else
installresources: .PHONY
.endif

all: ${APP_DIR} ${PROG} installresources

${APP_DIR}:
	@${ECHO} building ${APP_DIR} bundle
	mkdir -p "${APP_DIR}/Contents/Helium" \
		"${APP_DIR}/Contents/Resources"
	touch "${APP_DIR}/Contents/Info.plist"
	touch "${APP_DIR}/Contents/PkgInfo"

.include <bsd.prog.mk>
