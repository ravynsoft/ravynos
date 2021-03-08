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
_VERLIBDIR=
_LIBPATTERN=echo 
.for fmwk in ${FRAMEWORKS}
_INCDIRS+= -I${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/Helium\/..\//1}.framework/Headers
_LIBDIRS+= -L${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/Helium\/..\//1}.framework/Versions/Current
_LIBPATTERN+= ${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/Helium\/..\//1}.framework/Versions/Current/*.so  
_VERLIBDIR+= -Wl,--rpath=${fmwk:tW:S/$/.frameworks\/Versions\/${fmwk:tW:S/$/.framework\/Versions\/Current/:tA:T}/:S/..\//\$ORIGIN\/..\//1:Q}
.endfor

.for lib in ${_LIBPATTERN:sh}
_LIBS+= -l${lib:C%.*lib%%:C%\.so$%%}
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

.include <he.common.mk>

all: ${APP_DIR} ${PROG} installresources

${APP_DIR}:
	@${ECHO} building ${APP_DIR} bundle
	mkdir -p "${APP_DIR}/Contents/Helium" \
		"${APP_DIR}/Contents/Resources"
	touch "${APP_DIR}/Contents/Info.plist"
	touch "${APP_DIR}/Contents/PkgInfo"

.include <bsd.prog.mk>
