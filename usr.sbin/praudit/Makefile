#
#

.include <src.opts.mk>

OPENBSMDIR=${SRCTOP}/contrib/openbsm
.PATH: ${OPENBSMDIR}/bin/praudit

CFLAGS+=	-I${OPENBSMDIR}

PACKAGE=audit
PROG=	praudit
MAN=	praudit.1

WARNS?=	3

LIBADD=	bsm System notify

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

.include <bsd.prog.mk>
