APP_DIR=${APP}.app
NO_ROOT=yes
PROG=${APP_DIR}/Contents/ravynOS/${APP}
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
_FMWKDIRS=
_FMWKFLAG=
.for fmwk in ${FRAMEWORKS}
_LIBDIRS+= -L${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/ravynOS\/..\//1}.framework/Versions/Current
_LIBPATTERN+= ${fmwk:tW:S/^..\//${APP_DIR}\/Contents\/ravynOS\/..\//1}.framework/Versions/Current/*.so  
_VERLIBDIR+= -Wl,--rpath=${fmwk:tW:S/$/.framework\/Versions\/${fmwk:tW:S/$/.framework\/Versions\/Current/:tA:T}/:S/..\//\$ORIGIN\/..\//1:Q}
_FMWKDIRS+= -F${fmwk:H:u}
_FMWKFLAG+= -framework ${fmwk:T:u}
.endfor

.for lib in ${_LIBPATTERN:sh}
_LIBS+= -l${lib:C%.*lib%%:C%\.so$%%}
.endfor

LDFLAGS+= ${_LIBDIRS} ${_LIBS} -lobjc -L/usr/local/lib -lunwind ${_VERLIBDIR}
FMWK_FLAG+= ${_FMWKDIRS} ${_FMWKFLAG}
.endif

.if defined(RESOURCES) && !empty(RESOURCES)
RSCDIR=${APP_DIR}/Contents/Resources
installresources: ${RESOURCES}
	for name in ${RESOURCES}; do \
		if [ -d $$name ]; then \
			dest=${RSCDIR}/$$(basename $$name); \
			mkdir -p $$dest; \
			tar -C $$name -cf - . | tar -C $$dest -xvf -; \
		else \
			cp -fv $$name ${RSCDIR}/; \
		fi; \
	done
.else
installresources: .PHONY
.endif

.include <rvn.common.mk>

all: ${APP_DIR} ${PROG} installresources ${APP_DIR}/${APP}

${APP_DIR}/${APP}:
	ln -s Contents/ravynOS/${APP} ${APP_DIR}/${APP}

${APP_DIR}:
	@${ECHO} building ${APP_DIR} bundle
	mkdir -p "${APP_DIR}/Contents/ravynOS" \
		"${APP_DIR}/Contents/Resources"
	ln -s Contents/Resources "${APP_DIR}/Resources"
	if [ -f ${.CURDIR}/Info.plist ]; then \
		cp -fv ${.CURDIR}/Info.plist ${APP_DIR}/Contents; fi
	if [ -f ${.CURDIR}/PkgInfo ]; then \
		cp -fv ${.CURDIR}/PkgInfo ${APP_DIR}/Contents; fi

.include <bsd.prog.mk>
