FRAMEWORK_DIR=${FRAMEWORK}.framework
NO_ROOT=yes

UID != id -u
.if defined(UID) && ${UID} != 0
INSTALLFLAGS=-U
.endif

.if defined(SRCS) && !empty(SRCS)
LIBMODE?=0555
SHLIB_NAME=lib${FRAMEWORK}.so
SHLIB_MAJOR=A
SHLIBDIR=${FRAMEWORK_DIR}/Versions/A
.endif

.if defined(INCS) && !empty(INCS)
INSTALL+= ${INSTALLFLAGS}
INCLUDEDIR=${FRAMEWORK_DIR}/Versions/A/Headers
.endif

.if defined(RESOURCES) && !empty(RESOURCES)
RSCDIR=${FRAMEWORK_DIR}/Versions/A/Resources
installresources: ${RESOURCES}
	tar -C ${.CURDIR} -cf - ${RESOURCES} | tar -C ${RSCDIR} -xvf -
.else
installresources: .PHONY
.endif

all: ${FRAMEWORK_DIR} ${SHLIB_NAME} installincludes _libinstall installresources

${FRAMEWORK_DIR}:
	@${ECHO} building ${FRAMEWORK_DIR} bundle
	mkdir -p "${FRAMEWORK_DIR}/Versions/A/Headers" \
		"${FRAMEWORK_DIR}/Versions/A/Modules" \
		"${FRAMEWORK_DIR}/Versions/A/Resources"
	(cd "${FRAMEWORK_DIR}"; \
		ln -sf Versions/A/Headers Headers; \
		ln -sf Versions/A/Modules Modules; \
		ln -sf Versions/A/Resources Resources)
	(cd "${FRAMEWORK_DIR}/Versions"; ln -sf A Current)
	touch "${FRAMEWORK_DIR}/Versions/A/Resources/Info.plist"


.include <bsd.lib.mk>
.include <bsd.incs.mk>
