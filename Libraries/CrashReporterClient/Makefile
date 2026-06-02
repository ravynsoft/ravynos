LIB = CrashReporterClient
SRCS = CRClient.c CRClientCPP.cpp
OBJS = ${SRCS:S/.c$/.o/:S/.cpp$/.o/}
CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN}
LDFLAGS =

all: ${.OBJDIR}/lib${LIB}.dylib

${.OBJDIR}/lib${LIB}.a: ${OBJS}
	rm -f ${.TARGET}
	mkdir -p ${.TARGET:H} ${RAVYN_SDKROOT}/usr/lib
	${AR} rcs ${.TARGET} CRClientCPP.o CRClient.o
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

${.OBJDIR}/lib${LIB}.dylib: ${.OBJDIR}/lib${LIB}.a
	${CC} -o ${.TARGET} -shared -dylib -umbrella System -nostdlib \
	  -Wl,-install_name,/usr/lib/system/libCrashReporterClient.dylib \
	  -Wl,-current_version,1.0.0 \
	  -mmacos-version-min=${MACOS_VERSION_MIN} \
	  -Wl,-force_load,${.OBJDIR}/lib${LIB}.a ${LDFLAGS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/system/

.include <bsd.lib.mk>
