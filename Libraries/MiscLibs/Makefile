MK_WERROR = no

NV_SRCS = bsd_nvpair.c nvlist.c msgio.c
NV_OBJS = ${NV_SRCS:%.c=%.o}

SBUF_SRCS = sbuf.c
SBUF_OBJS = ${SBUF_SRCS:%.c=%.o}

CFLAGS = --sysroot=${RAVYN_SDKROOT} -I${.CURDIR} \
	 -mmacos-version-min=${MACOS_VERSION_MIN} \
	 -I${.CURDIR}/../openbsm/compat -D"__packed=__attribute__((packed))" \
	 -Wno-nullability-completeness

all: ${.OBJDIR}/libnv.a ${.OBJDIR}/libsbuf.a

${.OBJDIR}/libnv.a: ${NV_OBJS}
	${AR} rcs ${.TARGET} ${NV_OBJS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/

${.OBJDIR}/libsbuf.a: ${SBUF_OBJS}
	${AR} rcs ${.TARGET} ${SBUF_OBJS}
	cp -f ${.TARGET} ${RAVYN_SDKROOT}/usr/lib/

.include <bsd.lib.mk>
